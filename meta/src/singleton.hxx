#pragma once

#include <variant>

#include <p5/lambda/utils/container.hxx>

#include "api.hxx"
#include "log.hxx"
#include "bindings.hxx"
#include "namespace_.hxx"


namespace p5::refrigerator {
namespace singleton {

namespace parent_ = this_;
namespace this_ = parent_::singleton;

using Api = parent_::Api;
using Log = parent_::Log::Interface;
using Bindings = parent_::Bindings;
using Container = ::p5::lambda::utils::Container;

struct Type final {
    using Api = this_::Api;
    using Log = this_::Log;
    using Bindings = this_::Bindings;
    using Container = this_::Container;

    Api api;
    Log const &log;
    Bindings bindings;

    Container & container() noexcept(false);
    Container const & container() const noexcept(false);

    static Type & instance() noexcept(true);

private:
    parent_::Log log_;
    ::std::variant<::std::exception_ptr, Container> container_ = {};
    ::p5::lambda::utils::event::Subscription subscription_ = {};

    Type() noexcept(true);
    Type(Type &&) = delete;
    Type(Type const &) = delete;
    Type & operator = (Type &&) = delete;
    Type & operator = (Type const &) = delete;
};

} // namespace singleton

using Singleton = singleton::Type;

} // namespace p5::refrigerator
