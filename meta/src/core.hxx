#pragma once

#include <list>
#include <variant>
#include <optional>
#include <functional>

#include <p5/lambda/utils/container.hxx>

#include "api.hxx"
#include "log.hxx"
#include "bindings.hxx"
#include "namespace_.hxx"


namespace p5::refrigerator {
namespace core {

namespace parent_ = this_;
namespace this_ = parent_::core;

using Api = parent_::Api;
using Log = parent_::Log::Interface;
using Bindings = parent_::Bindings;
using Container = ::p5::lambda::utils::Container;

struct Module final {
    ::std::function<void(void)> load;
    ::std::function<void(void)> unload = {};
    bool critical = false;
};

struct Type final {
    using Api = this_::Api;
    using Log = this_::Log;
    using Module = this_::Module;
    using Bindings = this_::Bindings;
    using Container = this_::Container;

    Api api;
    Log const &log;
    Bindings bindings;

    Container & container() noexcept(false);
    Container const & container() const noexcept(false);

    void inject(Module &&) noexcept(false);
    void inject(Module const &) noexcept(false);

    void load() noexcept(false);
    void unload() noexcept(false);

    static Type & instance() noexcept(true);

private:
    parent_::Log log_;
    bool bad_ = false;
    bool lock_ = false;
    ::std::optional<::std::list<Module const *>> loaded_ = ::std::nullopt;
    ::std::list<Module> injected_ = {};
    ::std::variant<::std::exception_ptr, Container> container_ = {};
    ::p5::lambda::utils::event::Subscription subscription_ = {};

    Type() noexcept(true);
    Type(Type &&) = delete;
    Type(Type const &) = delete;
    Type & operator = (Type &&) = delete;
    Type & operator = (Type const &) = delete;
};

} // namespace core

using Core = core::Type;

} // namespace p5::refrigerator
