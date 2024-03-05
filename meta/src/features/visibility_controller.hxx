#pragma once

#include <memory>

#include "namespace_.hxx"
#include "visibility_controller/namespace_.hxx"


namespace p5::refrigerator::features {
namespace visibility_controller {

struct Type final {
    ~Type() noexcept(true);

private:
    struct Context_;

    ::std::shared_ptr<Context_> const context_;

    Type() = delete;
    Type(Type &&) = delete;
    Type(Type const &) = delete;

    Type & operator = (Type &&) = delete;
    Type & operator = (Type const &) = delete;

public:
    explicit Type(::std::shared_ptr<Context_> &&) noexcept(false);
};

} // namespace visibility_controller

using VisibilityController = visibility_controller::Type;

} // namespace p5::refrigerator::features
