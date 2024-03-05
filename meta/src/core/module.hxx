#pragma once

#include <utility>
#include <stdexcept>
#include <functional>
#include <type_traits>

#include <p5/lambda/utils/async/task.hxx>

#include "namespace_.hxx"


namespace p5::refrigerator::core {
namespace module {

namespace parent_ = this_;
namespace this_ = parent_::module;

struct Loader final {
    using Task = ::p5::lambda::utils::async::Task<void>;
    using Action = ::std::function<Task(void)>;

    operator bool () const noexcept(true);
    Task operator () () const noexcept(false);

    Loader(auto &&action) noexcept(false) requires(! ::std::is_base_of_v<
        Loader, ::std::decay_t<decltype(action)>
    >);

    Loader() noexcept(true);
    Loader(Loader &&) noexcept(true);
    Loader(Loader const &) noexcept(false);
    Loader & operator = (Loader &&) noexcept(true);
    Loader & operator = (Loader const &) noexcept(false);

private:
    Action action_ = {};
};

struct Type final {
    using Loader = this_::Loader;

    Loader load = {};
    ::std::function<void(void)> unload = {};
};

} // namespace module

using Module = this_::module::Type;

} // namespace p5::refrigerator::core


namespace p5::refrigerator::core::module {

inline this_::Loader::Loader(auto &&action) noexcept(false) requires(
    ! ::std::is_base_of_v<Loader, ::std::decay_t<decltype(action)>>
) {
    if constexpr (::std::is_void_v<decltype(action())>) {
        if (auto action_ = ::std::function<void(void)>{
            ::std::forward<decltype(action)>(action)
        }) this->action_ = [action_ = ::std::move(action_)] {
            action_(); return Task{};
        };
    }

    else action_ = ::std::forward<decltype(action)>(action);

    if (! action_) throw ::std::invalid_argument{
        "bad core module loader action"
    };
}

} // namespace p5::refrigerator::core::module
