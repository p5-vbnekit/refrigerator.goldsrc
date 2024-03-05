#pragma once

#include <p5/lambda/engine+fwd.hxx>
#include <p5/lambda/metamod+fwd.hxx>
#include <p5/lambda/utils/log/interface.hxx>

#include "namespace_.hxx"


namespace p5::refrigerator::core {
namespace log {

namespace parent_ = this_;
namespace this_ = parent_::log;

using Interface = ::p5::lambda::utils::log::Interface;

namespace implementation {

namespace parent_ = this_;
namespace this_ = parent_::implementation;

using Interface = parent_::Interface;

struct Context final {
    struct Meta final {
        using Api = ::p5::lambda::metamod::Functions;
        using Plugin = ::p5::lambda::metamod::plugin::Info;

        Api const &api;
        Plugin const &plugin;
    };

    using Engine = ::p5::lambda::engine::Functions;

    Meta meta;
    Engine const &engine;
};

struct Type final: this_::Interface {
    using Context = this_::Context;
    using Interface = this_::Interface;

    explicit Type(Context &&) noexcept(true);

private:
    Context context_;

    virtual void write_(
        Interface::Message &&
    ) const noexcept(true) override final;

    Type(Type &&) = delete;
    Type(Type const &) = delete;
    Type & operator = (Type &&) = delete;
    Type & operator = (Type const &) = delete;
};

} // namespace implementation

using Implementation = this_::implementation::Type;

} // namespace log

using Log = this_::log::Interface;

} // namespace p5::refrigerator::core
