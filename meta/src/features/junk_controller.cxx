#include <cstdint>

#include <list>
#include <tuple>
#include <memory>
#include <utility>
#include <optional>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <forward_list>

#include <p5/lambda/common.hxx>
#include <p5/lambda/engine.hxx>
#include <p5/lambda/utils/container/pointer.hxx>
#include <p5/lambda/utils/event/subscription.hxx>

#include "../core.hxx"
#include "../binding.hxx"
#include "../bindings.hxx"
#include "../exception.hxx"

#include "deity.hxx"
#include "junk_controller.hxx"
#include "trusted_entities.hxx"


namespace p5::refrigerator::features::junk_controller {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

} // namespace private_

struct Type::Context_ final {
    using DeityPointer = ::p5::lambda::utils::container::Pointer<
        parent_::Deity
    >;

    using Subscription = ::p5::lambda::utils::event::Subscription;

    DeityPointer const deity;

    ::std::optional<::std::size_t> limit = ::std::nullopt;
    ::std::list<this_::Item> assigned = {};

    ::std::forward_list<Subscription> bindings = {};

private:
    static void const * const injection_;

    static auto const * inject_() noexcept(true);
};

bool this_::Type::assign(Item item) noexcept(false) {
    if (! item) throw ::std::invalid_argument{"empty pointer"};
    auto const end_ = ::std::end(context_->assigned);
    if (end_ != ::std::find(
        ::std::begin(context_->assigned), end_, item)
    ) return false;
    context_->assigned.push_back(item);
    return true;
}

inline this_::Type::Type::~Type() noexcept(true) = default;

inline this_::Type::Type(::std::shared_ptr<Context_> &&context) noexcept(false):
    context_{::std::move(context)}
{
    if (! context_) throw ::std::invalid_argument{"empty context"};
}

namespace private_ {

namespace root = parent_::parent_::parent_;

inline static auto reset_context(auto &context) noexcept(true) {
    context.assigned.clear();
}

inline static auto on_leave_trusted(auto &context, auto const &entities) noexcept(true) {
    for (auto * const entity_: entities) context.assigned.remove(entity_);
}

inline static auto on_game_frame(auto const &engine, auto &context) noexcept(true) {
    if (! context.limit) context.limit = [
        &globals_ = engine.globals
    ] {
        using Value_ = ::std::decay_t<decltype(*(context.limit))>;
        if (! globals_) return static_cast<Value_>(0);
        auto const maximum_ = globals_->maxEntities;
        if (! (0 < maximum_)) return static_cast<Value_>(0);
        auto const reserved_ = ::std::max(
            static_cast<::std::decay_t<decltype(maximum_)>>(128), maximum_ / 4
        );
        if (! (reserved_ < maximum_)) return static_cast<Value_>(0);
        auto const value_ = maximum_ - reserved_;
        if (! (0 < value_)) return static_cast<Value_>(0);
        return static_cast<Value_>(value_);
    } ();

    if (context.assigned.empty()) return;

    if (! context.deity) return;
    auto &deity_ = *(context.deity);

    for (auto overage_ = [
        &limit_ = *(context.limit),
        &engine_ = engine.functions.numberOfEntities
    ] {
        auto value_ = ::std::numeric_limits<
            ::std::decay_t<decltype(limit_)>
        >::max();
        if (! (0 < limit_)) return value_;
        auto const engine_value_ = engine_();
        if (! (0 < engine_value_)) return value_;
        value_ = static_cast<::std::decay_t<decltype(value_)>>(engine_value_);
        if (value_ > limit_) return value_ - limit_;
        return static_cast<::std::decay_t<decltype(value_)>>(0);
    } (); 0 < overage_; --overage_) {
        auto * const entity_ = context.assigned.front();
        auto const &log_ = this_::root::Singleton::instance().log;
        log_.write() << "removing overage: " << entity_;
        try { deity_.remove_entity(entity_); }
        catch (...) {
            auto const &log_ = this_::root::Singleton::instance().log;
            log_.write<
                ::std::decay_t<decltype(log_)>::Message::Level::Warning
            >() << "failed to remove entity [" << entity_ << "]: "
            << this_::root::exception::generate_details();
        }
        if (context.assigned.empty()) break;
        if (entity_ == context.assigned.front()) {
            auto const &log_ = this_::root::Singleton::instance().log;
            log_.write<
                ::std::decay_t<decltype(log_)>::Message::Level::Warning
            >() << "entity [" << entity_ << "] not removed";
            context.assigned.pop_front();
            if (context.assigned.empty()) break;
        }
    }
}

} // namespace private_

auto const * this_::Type::Context_::inject_() noexcept(true) {
    namespace root_ = this_::private_::root;

    auto &singleton_ = root_::Singleton::instance();
    using LogLevel_ = ::std::decay_t<decltype(singleton_.log)>::Message::Level;

    auto &&injection_ = [&singleton_] {
        auto const &container_ = singleton_.container();

        auto const &trusted_ = singleton_.container().get<
            parent_::TrustedEntities const
        >(::std::rethrow_exception).resolve();

        auto &&context_ = [&container_] {
            auto &&deity_ = container_.get<parent_::Deity>(
                ::std::rethrow_exception
            );
            auto * const pointer_ = new this_::Type::Context_{
                .deity = ::std::move(deity_)
            };
            return ::std::shared_ptr<
                ::std::decay_t<decltype(*pointer_)>
            >{pointer_};
        } ();

        context_->bindings.push_front(::std::move(trusted_.on_exit([
            &context_ = *context_
        ] (auto const &entities) {
            this_::private_::on_leave_trusted(context_, entities);
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            root_::binding::Phase::After,
            root_::bindings::game::Init
        >([&context_ = *context_] () {
            this_::private_::reset_context(context_);
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            root_::binding::Phase::Before,
            root_::bindings::game::Frame
        >([&engine_ = singleton_.api.engine, &context_ = *context_] {
            this_::private_::on_game_frame(engine_, context_);
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            root_::binding::Phase::Before,
            root_::bindings::game::Shutdown
        >([&context_ = *context_] {
            this_::private_::reset_context(context_);
        }).take()));

        return ::std::make_tuple(::std::move(context_));
    };

    try {
        singleton_.container().inject<this_::Type>(
            ::std::in_place_t{}, ::std::move(injection_)
        );
    } catch(...) {
        singleton_.log.write<LogLevel_::Error>()
        << root_::exception::generate_details();
    }

    return static_cast<void const *>(nullptr);
}

void const * const this_::Type::Context_::injection_{
    this_::Type::Context_::inject_()
};

} // namespace p5::refrigerator::features::junk_controller
