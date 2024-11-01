#include <utility>
#include <exception>
#include <stdexcept>
#include <type_traits>

#include <boost/core/demangle.hpp>
#include <boost/algorithm/string.hpp>

#include "core.hxx"
#include "exception.hxx"


namespace p5::refrigerator::singleton {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

struct ScopedLock final {
    inline explicit ScopedLock(auto &flag) noexcept(false): flag_{flag} {
        if (flag) throw ::std::logic_error{"locked state"};
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

this_::Container & this_::Type::container() noexcept(false) {
    auto const index_ = container_.index();
    if (1 == index_) return ::std::get<1>(container_);
    if (0 == index_) if (
        auto const exception_ = ::std::get<0>(container_)
    ) ::std::rethrow_exception(exception_);
    throw ::std::logic_error{"invalid state"};
}

this_::Container const & this_::Type::container() const noexcept(false) {
    return const_cast<::std::decay_t<decltype(*this)> *>(this)->container();
}

void this_::Type::inject(Module &&module) noexcept(false) {
    if (bad_) throw ::std::logic_error{"bad state"};
    if (! module.load) throw ::std::invalid_argument{
        "bad module: loader is empty"
    };
    [[maybe_unused]] auto lock_ = this_::private_::ScopedLock{this->lock_};
    injected_.push_back(::std::move(module));
    if (! loaded_) return;
    auto &module_ = injected_.back();
    auto const iterator_ = loaded_->insert(::std::begin(*loaded_), nullptr);
    try { module_.load(); } catch (...) {
        loaded_->erase(iterator_);
        log_.write<
            ::std::decay_t<decltype(log_)>::Message::Level::Warning
        >() << parent_::exception::generate_details();
    }
}

void this_::Type::inject(Module const &module) noexcept(false) {
    inject(::std::decay_t<decltype(module)>{module});
}

void this_::Type::load() noexcept(false) {
    if (bad_) throw ::std::logic_error{"bad state"};
    if (loaded_) throw ::std::logic_error{"loaded already"};
    [[maybe_unused]] auto lock_ = this_::private_::ScopedLock{this->lock_};

    auto &&list_ = ::std::decay_t<decltype(*loaded_)>{};

    if (! injected_.empty()) try {
        for (auto const &module_: injected_) list_.push_back(&module_);
        auto iterator_ = ::std::begin(list_);
        auto const end_ = ::std::end(list_);
        do {
            auto const next_ = ::std::next(iterator_);
            auto const * const module_ = *iterator_;
            try { module_->load(); }
            catch (...) {
                list_.erase(iterator_);
                if (module_->critical) throw;
                log_.write<
                    ::std::decay_t<decltype(log_)>::Message::Level::Warning
                >() << parent_::exception::generate_details();
                continue;
            }
            iterator_ = next_;
        } while(end_ != iterator_);
    }

    catch (...) {
        for (auto const * const module_: list_) {
            if (! module_->unload) continue;
            try { module_->unload(); } catch (...) {
                bad_ = true;
                log_.write<
                    ::std::decay_t<decltype(log_)>::Message::Level::Warning
                >() << parent_::exception::generate_details();
            }
        }
        if (bad_) injected_.clear();
        throw;
    }

    list_.reverse();
    loaded_ = ::std::move(list_);
}

void this_::Type::unload() noexcept(false) {
    if (bad_) throw ::std::logic_error{"bad state"};
    if (! loaded_) throw ::std::logic_error{"not loaded"};
    [[maybe_unused]] auto lock_ = this_::private_::ScopedLock{this->lock_};

    auto list_ = ::std::move(*loaded_);
    loaded_ = ::std::nullopt;

    if (list_.empty()) return;

    for (auto const * const module_: list_) {
        if (! module_->unload) continue;
        try { module_->unload(); } catch (...) { bad_ = true; log_.write<
            ::std::decay_t<decltype(log_)>::Message::Level::Warning
        >() << parent_::exception::generate_details(); }
    }

    if (bad_) {
        injected_.clear();
        throw ::std::logic_error{"unable to unload some modules"};
    }
}

this_::Type & this_::Type::instance() noexcept(true) {
    static Type instance_; return instance_;
}

this_::Type::Type() noexcept(true):
    api{parent_::api::factory()}, log{log_}, bindings{}, log_{{
        .meta = {.api = api.meta.functions, .plugin = api.meta.plugin.info},
        .engine = api.engine.functions
    }}
{
    bindings.apply(api.meta.plugin.functions);

    using LogLevel_ = ::std::decay_t<decltype(log)>::Message::Level;

    try { container_.emplace<1>(); }

    catch (...) {
        auto const exception_ = ::std::current_exception();
        container_.emplace<0>(exception_);
        log.write<LogLevel_::Error>() << "container error: "
        << parent_::exception::generate_details(exception_);
        return;
    }

    auto &container_ = ::std::get<1>(this->container_);

    try { subscription_ = ::std::move(container_.on_error([this] (
        auto const &key, auto const &exception
    ) {
        log.write<LogLevel_::Error>() << "container error [" <<
        ::boost::core::demangle(key.name()) << "]: "
        << parent_::exception::generate_details(exception);
    }).take()); } catch (...) { log.write<LogLevel_::Error>(
        ::p5::lambda::utils::exception::details()
    ); }

    try { container_.assign(api); } catch (...) { log.write(
        LogLevel_::Error,
        parent_::exception::generate_details()
    ); }

    try { container_.assign(bindings); } catch (...) { log.write(
        LogLevel_::Error,
        parent_::exception::generate_details()
    ); }
}

} // namespace p5::refrigerator
