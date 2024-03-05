#include <stdexcept>
#include <type_traits>

#include "limit_.hxx"


namespace p5::refrigerator::features::visibility_controller::limit_ {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

// ignore less as false-positive
// 32 as a small reserve
// 256 from legacy goldsrc engine (should be increased)
// https://github.com/ValveSoftware/halflife/issues/12
inline constexpr static auto hardcoded_value_ = 256 - 32;

inline static auto gather(auto const &state) noexcept(true) {
    if ([&state] {
        if (! state.client) return false;
        if (! state.destination) return false;
        if (! (this_::hardcoded_value_ < state.accepted)) return false;
        if (! (this_::hardcoded_value_ < state.requested)) return false;
        if (! (this_::hardcoded_value_ < state.expectation)) return false;
        return state.requested < state.expectation;
    } ()) return state.accepted;
    return static_cast<::std::decay_t<decltype(state.accepted)>>(0);
}

inline static auto on_entity_requested(
    auto &context, auto accepted, auto const *client,
    auto index, auto const *destination
) noexcept(false) {
    if (! client) throw ::std::invalid_argument{"empty client pointer"};

    if (! (0 <= index)) throw ::std::invalid_argument{"negative entity index"};

    if (! destination) throw ::std::invalid_argument{
        "empty destination pointer"
    };

    if (! context.api) return;

    auto const api_limit_ = context.api->entities_limit();
    if (! (api_limit_ > index)) throw ::std::invalid_argument{
        "entity index too hight"
    };

    auto const expectation_ = [&] () -> ::std::decay_t<
        decltype(context.state.expectation)
    > {
        auto index_ = api_limit_;
        while (this_::hardcoded_value_ < (--index_)) {
            auto const * const entity_ = context.api->entity(index_);
            if (! entity_) continue;
            auto const &effects_ = entity_->variables.effects;
            using Effects_ = ::std::decay_t<decltype(effects_)>;
            using EffectsMask_ = ::std::underlying_type_t<Effects_>;
            constexpr static auto const mask_ = static_cast<
                EffectsMask_
            >(Effects_::NoDraw);
            if (mask_ & static_cast<EffectsMask_>(effects_)) continue;
            return static_cast<::std::decay_t<
                decltype(context.state.expectation)
            >>(index_);
        }
        return 0;
    } ();

    if ([&] {
        if (! (this_::hardcoded_value_ < expectation_)) return true;
        if (! (context.state.expectation <= expectation_)) return true;
        context.state.expectation = expectation_;
        return false;
    } ()) { context.api = nullptr; return; }

    auto const index_ = static_cast<
        ::std::decay_t<decltype(context.state.requested)>
    >(index);

    if (context.state.client == client) {
        if (context.state.xash) {
            if (context.state.destination != destination) context.api = nullptr;
            return;
        }

        else if (context.state.destination == destination) {
            context.state.xash = true;
            return;
        }

        if (index_ > context.state.requested) context.state.requested = index_;
    }

    else {
        auto const new_value_ = this_::gather(context.state);
        if (context.value < new_value_) context.value = new_value_;
        context.state = {.client = client, .requested = index_};
    }

    if (accepted) {
        context.state.accepted += 1;
        context.state.destination = destination;
    }
}

} // namespace private_

this_::Value this_::Type::value() const noexcept(false) {
    if (! context_) throw ::std::logic_error{"bad state"};
    return context_->value;
}

void this_::Type::on_new_frame(Api const &api) noexcept(false) {
    if (! context_) throw ::std::logic_error{"bad state"};
    if (context_->api) {
        auto const new_value_ = this_::private_::gather(context_->state);
        if (context_->value < new_value_) context_->value = new_value_;
    }
    context_->api = &api;
    context_->state = {};
}

void this_::Type::on_entity_requested(
    bool accepted, void const *client, int index, void const *destination
) noexcept(false) {
    if (! context_) throw ::std::logic_error{"bad state"};
    this_::private_::on_entity_requested(
        *context_, accepted, client, index, destination
    );
}

} // namespace p5::refrigerator::features::visibility_controller::limit_
