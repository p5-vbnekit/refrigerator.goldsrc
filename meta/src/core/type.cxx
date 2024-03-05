#include <utility>
#include <iterator>
#include <optional>
#include <exception>
#include <stdexcept>
#include <type_traits>

#include "type.hxx"
#include "../exception.hxx"


namespace p5::refrigerator::core {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

struct ScopedLock final {
    inline explicit ScopedLock(auto &flag) noexcept(false): flag_{flag} {
        if (flag) throw ::std::logic_error{"bad core state: locked"};
    }

    inline ~ScopedLock() noexcept(true) { flag_ = false; }

private:
    bool &flag_;

    ScopedLock() = delete;
    ScopedLock(ScopedLock &&) = delete;
    ScopedLock(ScopedLock const &) = delete;

    ScopedLock & operator = (ScopedLock &&) = delete;
    ScopedLock & operator = (ScopedLock const &) = delete;
};

} // namespace private_

this_::Bindings * this_::Type::bindings() noexcept(true) {
    if (bindings_) return &(*bindings_);
    return nullptr;
}

this_::Container * this_::Type::container() noexcept(true) {
    if (container_) return &(*container_);
    return nullptr;
}

this_::Container const * this_::Type::container() const noexcept(true) {
    if (container_) return &(*container_);
    return nullptr;
}

void this_::Type::inject(Module &&module) noexcept(false) {
    if (bad_) throw ::std::logic_error{"bad core state"};
    if (container_) throw ::std::logic_error{"bad core state: loaded"};
    [[maybe_unused]] auto lock_ = this_::private_::ScopedLock{this->lock_};
    injected_.emplace_back(::std::move(module));
}

void this_::Type::inject(Module const &module) noexcept(false) {
    inject(::std::decay_t<decltype(module)>{module});
}

void this_::Type::load() noexcept(false) {
    if (bad_) throw ::std::logic_error{"bad core state"};
    if (container_) throw ::std::logic_error{"bad core state: loaded"};
    [[maybe_unused]] auto const lock_ = this_::private_::ScopedLock{
        this->lock_
    };

    auto &&loaded_ = ::std::decay_t<decltype(this->loaded_)>{};
    auto tasks_ = ::std::list<
        ::std::decay_t<decltype(loaded_.front()->load())>
    >{};

    using LogLevel_ = ::std::decay_t<decltype(log_)>::Message::Level;

    try {
        bindings_.emplace();
        container_.emplace();
        container_->assign(api);
        container_->assign(&(*bindings_));
        if (! injected_.empty()) {
            auto queue_ = ::std::decay_t<decltype(loaded_)>{};
            for (auto const &module_: injected_) queue_.push_back(&module_);
            auto iterator_ = ::std::begin(queue_);
            auto const end_ = ::std::end(queue_);
            do {
                auto const next_ = ::std::next(iterator_);
                auto const * const module_ = *iterator_;
                auto &&task_ = module_->load();
                if (task_.linked()) {
                    auto const &future_ = task_.future();
                    future_.subscribe([
                        future_, &log_ = log_,
                        &loaded_, iterator_, &queue_
                    ] {
                        using State_ = ::std::decay_t<
                            decltype(future_.state())
                        >;
                        if (State_::Cancelled == future_.state()) return;
                        try { future_.get(::std::rethrow_exception); }
                        catch (...) {
                            log_.write<LogLevel_::Warning>()
                            << parent_::exception::generate_details();
                            return;
                        }
                        loaded_.splice(
                            ::std::begin(loaded_), queue_, iterator_
                        );
                    }).pin();
                    tasks_.push_back(::std::move(task_));
                }
                else loaded_.splice(::std::begin(loaded_), queue_, iterator_);
                iterator_ = next_;
            } while(end_ != iterator_);
            if (! tasks_.empty()) {
                auto stuck_ = false;
                for (auto const &task_: tasks_) {
                    auto const &future_ = task_.future();
                    if (future_.exception()) throw ::std::runtime_error{
                        "async module loading exception occured"
                    };
                    if (! task_.future().get()) stuck_ = true;
                }
                if (stuck_) throw ::std::logic_error{
                    "async module loading stuck"
                };
            }
        }
    }

    catch (...) {
        if (! tasks_.empty()) {
            for (auto const &task_: tasks_) task_.stop();
            tasks_.clear();
        }
        for (auto const * const module_: loaded_) {
            if (! module_->unload) continue;
            try { module_->unload(); } catch (...) {
                bad_ = true;
                log_.write<LogLevel_::Warning>()
                << parent_::exception::generate_details();
            }
        }
        auto container_ = ::std::exchange(this->container_, ::std::nullopt);
        bindings_.reset();
        container_.reset();
        if (bad_) injected_.clear();
        throw;
    }

    this->loaded_ = ::std::move(loaded_);
}

void this_::Type::unload() noexcept(false) {
    if (! container_) throw ::std::logic_error{"bad core state: not loaded"};
    [[maybe_unused]] auto const lock_ = this_::private_::ScopedLock{
        this->lock_
    };

    auto exception_ = ::std::optional<::std::exception_ptr>{::std::nullopt};

    if (! loaded_.empty()) try {
        auto failed_ = false;
        auto const loaded_ = ::std::move(this->loaded_);

        for (auto const * const module_: loaded_) {
            if (! module_->unload) continue;
            try { module_->unload(); } catch (...) { failed_ = true; log_.write<
                ::std::decay_t<decltype(log_)>::Message::Level::Warning
            >() << parent_::exception::generate_details(); }
        }

        if ((bad_ = failed_)) throw ::std::logic_error{
            "unable to unload some modules"
        };
    }

    catch (...) { exception_.emplace(::std::current_exception()); }

    auto container_ = ::std::exchange(this->container_, ::std::nullopt);
    bindings_.reset();
    container_.reset();
    if (bad_) injected_.clear();

    if (! exception_) return;
    if (*(exception_)) ::std::rethrow_exception(*exception_);
    throw ::std::bad_exception{};
}

void this_::Type::invalidate() noexcept(true) {
    if (bad_) return;
    bad_ = true;
    if (lock_ || container_) return;
    injected_.clear();
}

this_::Type & this_::Type::instance() noexcept(true) {
    static Type instance_; return instance_;
}

this_::Type::Type() noexcept(true):
    api{this_::api::factory()}, log{log_}, log_{{
        .meta = {.api = api.meta.functions, .plugin = api.meta.plugin.info},
        .engine = api.engine.functions
    }}
{
    ::std::decay_t<decltype(*bindings_)>::apply(api.meta.plugin.functions);
}

} // namespace p5::refrigerator::core
