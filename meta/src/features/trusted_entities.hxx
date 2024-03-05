#pragma once

#include <list>
#include <memory>
#include <functional>
#include <type_traits>

#include <p5/lambda/common+fwd.hxx>
#include <p5/lambda/utils/async/subscription+fwd.hxx>

#include "namespace_.hxx"


namespace p5::refrigerator::features {
namespace trusted_entities {

namespace parent_ = this_;
namespace this_ = parent_::trusted_entities;

using Item = ::p5::lambda::common::entity::Dictionary *;
using List = ::std::list<this_::Item>;
using Handler = ::std::function<void(this_::List const &)>;
using Subscription = ::p5::lambda::utils::async::Subscription;

struct Type final {
    using Item = this_::Item;
    using List = this_::List;
    using Handler = this_::Handler;
    using Subscription = this_::Subscription;

    List const & queue() const noexcept(true);
    List const & entered() const noexcept(true);

    bool insert(Item) noexcept(false);
    bool remove(Item) noexcept(false);

    Subscription on_exit(Handler &&) const noexcept(false);
    Subscription on_enter(Handler &&) const noexcept(false);

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

} // namespace trusted_entities

using TrustedEntities = trusted_entities::Type;

} // namespace p5::refrigerator::features
