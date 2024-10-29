#pragma once

#include <memory>
#include <stdexcept>
#include <functional>

#include <p5/lambda/common+fwd.hxx>
#include <p5/lambda/utils/event/subscription.hxx>

#include "namespace_.hxx"


namespace p5::refrigerator::features {
namespace client_variables {

namespace parent_ = this_;
namespace this_ = parent_::client_variables;

using Entity = ::p5::lambda::common::entity::Dictionary;
using Handler = ::std::function<void(char const *)>;
using Subscription = ::p5::lambda::utils::event::Subscription;

struct Type final {
    using Entity = this_::Entity;
    using Handler = this_::Handler;
    using Subscription = this_::Subscription;

    auto query(
        auto const *client, auto const *name, auto &&handler
    ) noexcept(false);

    ~Type() noexcept(true);

private:
    struct Context_;

    ::std::shared_ptr<Context_> const context_;

    Subscription query_(
        Entity const *, char const *, Handler &&
    ) noexcept(false);

    Type() = delete;
    Type(Type &&) = delete;
    Type(Type const &) = delete;

    Type & operator = (Type &&) = delete;
    Type & operator = (Type const &) = delete;

public:
    explicit Type(::std::shared_ptr<Context_> &&) noexcept(false);
};

} // namespace client_variables

using ClientVariables = client_variables::Type;

namespace client_variables {

inline auto this_::Type::query(
    auto const *client, auto const *name, auto &&handler
) noexcept(false) {
    if (! client) throw ::std::invalid_argument{"empty client pointer"};
    if (! name) throw ::std::invalid_argument{"empty name"};
    query_(client, name, ::std::forward<decltype(handler)>(handler));
}

} // namespace client_variables
} // namespace p5::refrigerator::features
