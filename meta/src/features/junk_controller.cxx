#include <cstdint>

#include <list>
#include <limits>
#include <utility>
#include <iterator>
#include <optional>
#include <algorithm>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <forward_list>

#include <p5/lambda/common.hxx>
#include <p5/lambda/engine.hxx>
#include <p5/lambda/utils/async/subscription.hxx>

#include "../core.hxx"
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
    using Deity = parent_::Deity;
    using Subscription = ::p5::lambda::utils::async::Subscription;

    Deity &deity;

    ::std::optional<::std::size_t> limit = ::std::nullopt;
    ::std::list<this_::Item> assigned = {};

    ::std::forward_list<Subscription> subscriptions = {};

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

inline this_::Type::Type(
    ::std::decay_t<decltype(context_)> &&context
) noexcept(false):
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
        auto const maximum_ = globals_->max_entities;
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
        auto const &log_ = this_::root::Core::instance().log;
        log_.write() << "removing overage: " << entity_;
        try { context.deity.remove_entity(entity_); }
        catch (...) {
            auto const &log_ = this_::root::Core::instance().log;
            log_.write<
                ::std::decay_t<decltype(log_)>::Message::Level::Warning
            >() << "failed to remove entity [" << entity_ << "]: "
            << this_::root::exception::generate_details();
        }
        if (context.assigned.empty()) break;
        if (entity_ == context.assigned.front()) {
            auto const &log_ = this_::root::Core::instance().log;
            log_.write<
                ::std::decay_t<decltype(log_)>::Message::Level::Warning
            >() << "entity [" << entity_ << "] not removed";
            context.assigned.pop_front();
            if (context.assigned.empty()) break;
        }
    }
}

} // namespace private_

inline auto const * this_::Type::Context_::inject_() noexcept(true) {
    namespace root_ = private_::root;

    auto &core_ = root_::Core::instance();
    using LogLevel_ = ::std::decay_t<decltype(core_.log)>::Message::Level;

    try { core_.inject({.load = [&core_] () -> ::std::decay_t<
        decltype(core_)
    >::Module::Loader::Task {
        auto * const bindings_ = core_.bindings();
        if (! bindings_) throw ::std::logic_error{
            "bad core state: empty bindings pointer"
        };

        auto * const container_ = core_.container();
        if (! container_) throw ::std::logic_error{
            "bad core state: empty container pointer"
        };

        auto &&context_ = ::std::decay_t<decltype(this_::Type::context_)>{};

        auto &deity_ = co_await core_.container()->get<
            ::std::remove_reference_t<decltype(context_->deity)>
        >();

        auto const &trusted_ = co_await core_.container()->get<
            parent_::TrustedEntities const
        >();

        context_.reset(new ::std::decay_t<decltype(*context_)>{
            .deity = deity_
        });

        context_->subscriptions.push_front(trusted_.on_exit([
            &context_ = *context_
        ] (auto const &entities) {
            this_::private_::on_leave_trusted(context_, entities);
        }));

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::After,
            root_::core::bindings::game::Init
        >([&context_ = *context_] () {
            this_::private_::reset_context(context_);
        }));

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::Before,
            root_::core::bindings::game::StartFrame
        >([&engine_ = core_.api.engine, &context_ = *context_] {
            this_::private_::on_game_frame(engine_, context_);
        }));

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::Before,
            root_::core::bindings::game::Shutdown
        >([&context_ = *context_] {
            this_::private_::reset_context(context_);
        }));

        container_->emplace<this_::Type>(::std::move(context_)).pin();
    }}); } catch (...) {
        core_.invalidate();
        core_.log.write<LogLevel_::Error>()
        << parent_::parent_::exception::generate_details();
    }

    return static_cast<void const *>(nullptr);
}

void const * const this_::Type::Context_::injection_{
    this_::Type::Context_::inject_()
};

} // namespace p5::refrigerator::features::junk_controller
