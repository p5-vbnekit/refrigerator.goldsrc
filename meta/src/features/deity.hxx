#pragma once

#include <memory>
#include <type_traits>

#include <p5/lambda/common+fwd.hxx>

#include "namespace_.hxx"


namespace p5::refrigerator::features {
namespace deity {

namespace parent_ = this_;
namespace this_ = parent_::deity;

using Entity = ::p5::lambda::common::entity::Dictionary;

struct Type final {
    using Entity = this_::Entity;

    Entity * create_entity(char const * = nullptr) noexcept(false);
    void remove_entity(Entity const *) noexcept(false);

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

} // namespace deity

using Deity = deity::Type;

} // namespace p5::refrigerator::features
