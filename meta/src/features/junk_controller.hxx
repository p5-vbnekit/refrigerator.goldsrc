#pragma once

#include <memory>

#include <p5/lambda/common+fwd.hxx>

#include "namespace_.hxx"


namespace p5::refrigerator::features {
namespace junk_controller {

namespace parent_ = this_;
namespace this_ = parent_::junk_controller;

using Item = ::p5::lambda::common::entity::Dictionary *;

struct Type final {
    using Item = this_::Item;

    bool assign(Item entity) noexcept(false);

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

} // namespace junk_controller

using JunkController = junk_controller::Type;

} // namespace p5::refrigerator::features
