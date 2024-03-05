#pragma once

#include <list>
#include <memory>
#include <functional>

#include <p5/lambda+fwd.hxx>

#include "namespace_.hxx"


namespace p5::refrigerator::features {
namespace trusted_entities {

namespace parent_ = this_;
namespace this_ = parent_::trusted_entities;

using List = ::std::list<::p5::lambda::common::entity::Dictionary *>;
using Handler = ::std::function<void(this_::List const)>;
using Subscription = ::p5::lambda::utils::event::Subscription;

struct Type final {
    using List = this_::List;
    using Handler = this_::Handler;
    using Subscription = this_::Subscription;

    List const & state() const noexcept(true);
    Subscription on_exit(Handler &&) const noexcept(false);
    Subscription on_enter(Handler &&) const noexcept(false);

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

} // namespace report

using TrustedEntities = trusted_entities::Type;

} // namespace p5::refrigerator::features
