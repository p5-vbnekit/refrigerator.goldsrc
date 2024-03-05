#pragma once

#include <memory>

#include <p5/lambda/common+fwd.hxx>

#include "namespace_.hxx"


namespace p5::refrigerator::features {
namespace deity {

namespace parent_ = this_;
namespace this_ = parent_::deity;

using EntityPointer = ::p5::lambda::common::entity::Dictionary *;

struct Type final {
    using EntityPointer = this_::EntityPointer;

    EntityPointer create_entity(char const * = nullptr) noexcept(false);
    void remove_entity(EntityPointer) noexcept(false);

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

} // namespace deity

using Deity = deity::Type;

} // namespace p5::refrigerator::features
