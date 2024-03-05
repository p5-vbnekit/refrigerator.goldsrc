#pragma once

#include <tuple>
#include <memory>
#include <utility>
#include <optional>
#include <typeindex>
#include <functional>
#include <type_traits>

#include <p5/lambda/metamod+fwd.hxx>
#include <p5/lambda/utils/async/subscription.hxx>

#include "binding.hxx"
#include "namespace_.hxx"


namespace p5::refrigerator::core {
namespace bindings {

namespace parent_ = this_;
namespace this_ = parent_::bindings;

namespace game = parent_::binding::targets::game;
namespace engine = parent_::binding::targets::engine;

using Phase = parent_::binding::Phase;
using Injection = ::p5::lambda::utils::async::Subscription;
using Endpoints = ::p5::lambda::metamod::plugin::Functions;

template <Phase p> using Meta = parent_::binding::action::result::Meta<p>;

struct Type final {
    using Phase = this_::Phase;
    using Injection = this_::Injection;
    using Endpoints = this_::Endpoints;

    template <Phase phase> using Meta = this_::Meta<phase>;

    template <Phase, class, class Action>
    auto inject(Action &&) noexcept(false);

    static void apply(Endpoints &) noexcept(true);

    Type() noexcept(false);
    ~Type() noexcept(true);

private:
    struct Private_;
    ::std::unique_ptr<Private_> private_;

    Injection inject_(::std::type_index &&, ::std::function<
        Meta<Phase::Before>(void *, void const *)
    > &&) noexcept(false);

    Injection inject_(::std::type_index &&, ::std::function<
        Meta<Phase::After>(void *, void const *)
    > &&) noexcept(false);

    Type(Type &&) = delete;
    Type(Type const &) = delete;
    Type & operator = (Type &&) = delete;
    Type & operator = (Type const &) = delete;
};

} // namepsace bindings

using Bindings = this_::bindings::Type;

namespace bindings {

template <
    parent_::binding::Phase phase, class Target, class Action
> inline auto this_::Type::inject(Action &&action) noexcept(false) {
    using Traits_ = parent_::binding::action::Traits<phase, Target>;
    return this->inject_(parent_::binding::target::key<Target>(), [
        action_ = ::std::forward<Action>(action)
    ] (void *output, void const *input) {
        using Input_ = typename Traits_::Input;
        using Output_ = ::std::decay_t<decltype(::std::apply(
            action_, ::std::declval<Input_>()
        ))>;
        using Result_ = typename Traits_::Result;
        using Meta_ = typename Result_::Meta;
        auto const &input_ = *static_cast<Input_ const *>(input);
        if constexpr (::std::is_void_v<Output_>) {
            ::std::apply(action_, input_);
            return Meta_::Handled;
        }
        else {
            auto const result_ = ::std::apply(action_, input_);
            if constexpr(parent_::binding::action::is_result<Output_>()) {
                if constexpr (requires () { result_.value; }) *static_cast<
                    ::std::optional<typename Result_::Value> *
                >(output) = result_.value;
                return result_.meta;
            }
            else if constexpr(::std::is_same_v<Meta_, Output_>) return result_;
            else {
                *static_cast<
                    ::std::optional<typename Result_::Value> *
                >(output) = result_;
                return Meta_::Handled;
            }
        }
    });
}

} // namespace bindings
} // namespace p5::refrigerator::core
