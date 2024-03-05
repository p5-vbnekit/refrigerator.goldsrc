#pragma once

#include <memory>
#include <type_traits>

#include "namespace_.hxx"
#include "butchering/namespace_.hxx"


namespace p5::refrigerator::features {
namespace butchering {

struct Type final {
    ~Type() noexcept(true);

private:
    struct Context_;

    ::std::unique_ptr<Context_> const context_;

    Type() = delete;
    Type(Type &&) = delete;
    Type(Type const &) = delete;

    Type & operator = (Type &&) = delete;
    Type & operator = (Type const &) = delete;

public:
    explicit Type(::std::decay_t<decltype(context_)> &&) noexcept(false);
};

} // namespace butchering

using Butchering = butchering::Type;

} // namespace p5::refrigerator::features
