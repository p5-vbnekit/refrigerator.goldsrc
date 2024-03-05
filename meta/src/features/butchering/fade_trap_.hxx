#pragma once

#include <optional>
#include <functional>
#include <type_traits>
#include <unordered_map>

#include <p5/lambda/common.hxx>
#include <p5/lambda/utils/async/channel.hxx>
#include <p5/lambda/utils/async/subscription.hxx>

#include "namespace_.hxx"


namespace p5::refrigerator::features::butchering {
namespace fade_trap_ {

namespace parent_ = this_;
namespace this_ = parent_::fade_trap_;

using Pointer = ::p5::lambda::common::entity::Dictionary *;
using Handler = ::std::function<void(Pointer)>;
using Subscription = ::p5::lambda::utils::async::Subscription;

struct Type final {
    using Pointer = this_::Pointer;
    using Handler = this_::Handler;
    using Subscription = this_::Subscription;

    bool state(Pointer) const noexcept(true);
    void reset() noexcept(true);

    bool attach(Pointer) noexcept(false);
    bool detach(Pointer) noexcept(true);

    void after_think(Pointer) noexcept(false);
    void before_think(Pointer) noexcept(false);

    Subscription on_caught(Handler &&) const noexcept(false);

private:
    using Solid_ = ::std::decay_t<decltype(
        ::std::declval<Pointer>()->variables.solid
    )>;

    using RenderMode_ = ::std::decay_t<decltype(
        ::std::declval<Pointer>()->variables.render_mode
    )>;

    using RenderAmount_ = ::std::decay_t<decltype(
        ::std::declval<Pointer>()->variables.render_amount
    )>;

    struct ItemState_ final {
        struct Cache_ final {
            Solid_ solid = Solid_::Not;
            RenderMode_ render_mode = RenderMode_::Normal;
            RenderAmount_ render_amount = 0;
        };

        bool caught = false;
        ::std::optional<Cache_> cache = ::std::nullopt;
    };

    struct ThinkState_ final {
        Pointer pointer = nullptr;
        ::std::optional<::std::decay_t<
            decltype(*::std::declval<Pointer>())
        >> cache = ::std::nullopt;
    };

    ::std::unordered_map<Pointer, ItemState_> attached_ = {};
    mutable ::p5::lambda::utils::async::channel::Issuer<Pointer> channel_ = {};
    ThinkState_ think_state_ = {};
};

} // namespace fade_trap_

using FadeTrap_ = fade_trap_::Type;

} // namespace p5::refrigerator::features::butchering
