#pragma once

#include <p5/lambda/game+fwd.hxx>
#include <p5/lambda/engine+fwd.hxx>
#include <p5/lambda/common+fwd.hxx>
#include <p5/lambda/metamod+fwd.hxx>

#include "namespace_.hxx"


namespace p5::refrigerator {

void inject(
    ::p5::lambda::engine::Functions const *,
    ::p5::lambda::engine::Globals *
) noexcept(true);

::p5::lambda::common::IntegerBoolean query(
    char const *,
    ::p5::lambda::metamod::plugin::Info **,
    ::p5::lambda::metamod::Functions const *
) noexcept(true);

::p5::lambda::common::IntegerBoolean attach(
    ::p5::lambda::metamod::plugin::LoadTime,
    ::p5::lambda::metamod::plugin::Functions *,
    ::p5::lambda::metamod::Globals *,
    ::p5::lambda::game::functions::Pointers const *
) noexcept(true);

::p5::lambda::common::IntegerBoolean detach(
    ::p5::lambda::metamod::plugin::LoadTime,
    ::p5::lambda::metamod::plugin::UnloadReason
) noexcept(true);

} // namespace p5::refrigerator
