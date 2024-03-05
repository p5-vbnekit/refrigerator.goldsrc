#pragma once

#include <p5/lambda/game.hxx>
#include <p5/lambda/engine.hxx>
#include <p5/lambda/metamod.hxx>

#include "namespace_.hxx"


namespace p5::refrigerator::core {
namespace api {

namespace parent_ = this_;
namespace this_ = parent_::api;

struct Meta final {
    using Globals = ::p5::lambda::metamod::Globals;
    using Functions = ::p5::lambda::metamod::Functions;

    struct Plugin final {
        using Info = ::p5::lambda::metamod::plugin::Info;
        using Functions = ::p5::lambda::metamod::plugin::Functions;

        Info info = {};
        Functions functions = {};
    };

    Plugin plugin = {};
    Functions functions = {};
    Globals *globals = nullptr;
};

struct Game final {
    using Functions = ::p5::lambda::game::Functions;

    Functions functions = {};
};

struct Engine final {
    using Globals = ::p5::lambda::engine::Globals;
    using Functions = ::p5::lambda::engine::Functions;

    Functions functions = {};
    Globals *globals = nullptr;
};

struct Type final {
    using Engine = this_::Engine;
    using Game = this_::Game;
    using Meta = this_::Meta;

    Meta meta = {};
    Game game = {};
    Engine engine = {};
};

this_::Type factory() noexcept(true);

} // namepsace api

using Api = this_::api::Type;

} // namespace p5::refrigerator::core
