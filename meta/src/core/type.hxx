#pragma once

#include <list>
#include <optional>

#include "api.hxx"
#include "log.hxx"
#include "module.hxx"
#include "bindings.hxx"
#include "container.hxx"
#include "namespace_.hxx"


namespace p5::refrigerator::core {

struct Type final {
    using Api = this_::Api;
    using Log = this_::Log;
    using Module = this_::Module;
    using Bindings = this_::Bindings;
    using Container = this_::Container;

    Api api;
    Log const &log;

    Bindings * bindings() noexcept(true);
    Container * container() noexcept(true);
    Container const * container() const noexcept(true);

    void inject(Module &&) noexcept(false);
    void inject(Module const &) noexcept(false);

    void load() noexcept(false);
    void unload() noexcept(false);
    void invalidate() noexcept(true);

    static Type & instance() noexcept(true);

private:
    this_::log::Implementation log_;
    bool bad_ = false, lock_ = false;
    ::std::list<Module const *> loaded_ = {};
    ::std::list<Module> injected_ = {};
    ::std::optional<Bindings> bindings_ = ::std::nullopt;
    ::std::optional<Container> container_ = ::std::nullopt;

    Type() noexcept(true);
    Type(Type &&) = delete;
    Type(Type const &) = delete;
    Type & operator = (Type &&) = delete;
    Type & operator = (Type const &) = delete;
};

} // namespace p5::refrigerator::core
