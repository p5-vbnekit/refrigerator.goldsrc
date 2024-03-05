#pragma once

#include <tuple>
#include <typeindex>
#include <type_traits>

#include <p5/lambda/game.hxx>
#include <p5/lambda/engine.hxx>

#include "namespace_.hxx"


namespace p5::refrigerator::core {
namespace binding {

namespace parent_ = this_;
namespace this_ = parent_::binding;

namespace phase {

namespace parent_ = this_;
namespace this_ = parent_::phase;

enum class Type { Before, After };

template <Type> struct Traits final {};

template <> struct Traits<Type::Before> final {
    enum class Meta { Ignored, Handled, Override, Supercede };
};

template <> struct Traits<Type::After> final {
    enum class Meta { Ignored, Handled, Override };
};

} // namespace phase

using Phase = this_::phase::Type;

namespace targets {

namespace parent_ = this_;
namespace this_ = parent_::targets;

namespace game {

namespace parent_ = this_;
namespace this_ = parent_::game;

namespace related_ = ::p5::lambda::game;

struct Init final {
    using Prototype = ::std::remove_reference_t<decltype(
        *(related_::Functions::Standard::gameInit)
    )>;
};

struct Spawn final {
    using Prototype = ::std::remove_reference_t<decltype(
        *(related_::Functions::Standard::spawn)
    )>;
};

struct Think final {
    using Prototype = ::std::remove_reference_t<decltype(
        *(related_::Functions::Standard::think)
    )>;
};

struct ClientCommand final {
    using Prototype = ::std::remove_reference_t<decltype(
        *(related_::Functions::Standard::clientCommand)
    )>;
};

struct StartFrame final {
    using Prototype = ::std::remove_reference_t<decltype(
        *(related_::Functions::Standard::startFrame)
    )>;
};

struct AddToFullPack final {
    using Prototype = ::std::remove_reference_t<decltype(
        *(related_::Functions::Standard::addToFullPack)
    )>;
};

struct Shutdown final {
    using Prototype = ::std::remove_reference_t<decltype(
        *(related_::Functions::Extension::gameShutdown)
    )>;
};

} // namespace game

namespace engine {

namespace parent_ = this_;
namespace this_ = parent_::game;

namespace related_ = ::p5::lambda::engine;

struct CreateEntity final {
    using Prototype = ::std::remove_reference_t<decltype(
        *(related_::Functions::createEntity)
    )>;
};

struct RemoveEntity final {
    using Prototype = ::std::remove_reference_t<decltype(
        *(related_::Functions::removeEntity)
    )>;
};

struct CreateNamedEntity final {
    using Prototype = ::std::remove_reference_t<decltype(
        *(related_::Functions::createNamedEntity)
    )>;
};

} // namespace engine
} // namespace targets

namespace target {

namespace parent_ = this_;
namespace this_ = parent_::target;

namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

template <class Type, class ... Types> inline consteval static
auto is_one_of() noexcept(true) {
    return (::std::is_same_v<Type, Types> || ...);
}

} // namespace private_

template <class T> inline consteval static auto is_game() noexcept(true) {
    return this_::private_::is_one_of<T,
        parent_::targets::game::Init,
        parent_::targets::game::Spawn,
        parent_::targets::game::Think,
        parent_::targets::game::ClientCommand,
        parent_::targets::game::StartFrame,
        parent_::targets::game::AddToFullPack,
        parent_::targets::game::Shutdown
    >();
}

template <class T> inline consteval static auto is_engine() noexcept(true) {
    return this_::private_::is_one_of<T,
        parent_::targets::engine::CreateEntity,
        parent_::targets::engine::RemoveEntity,
        parent_::targets::engine::CreateNamedEntity
    >();
}

template <class T> inline consteval static auto is_known() noexcept(true) {
    return this_::is_game<T>() || this_::is_engine<T>();
}

template <class T> inline auto key() noexcept(true) {
    static_assert(::std::is_class_v<T>);
    static_assert(this_::is_known<T>());
    return ::std::type_index{typeid(T)};
}

template <class T> struct PrototypeTratis final {
    static_assert(::std::is_function_v<T>);
};

template <class O, class ... I> struct PrototypeTratis<O(I ...)> final {
    using Input = ::std::tuple<::std::remove_reference_t<I> & ...>;
    using Output = O;
};

template <class T> struct Traits final {
    static_assert(::std::is_class_v<T>);
    static_assert(this_::is_known<T>());

    using Prototype = typename T::Prototype;
    using Input = typename this_::PrototypeTratis<Prototype>::Input;
    using Output = typename this_::PrototypeTratis<Prototype>::Output;
};

} // namespace target

namespace action {

namespace parent_ = this_;
namespace this_ = parent_::action;

namespace result {

namespace parent_ = this_;
namespace this_ = parent_::result;

template <parent_::parent_::Phase phase>
using Meta = typename parent_::parent_::phase::Traits<phase>::Meta;

template <parent_::parent_::Phase p, class T = void> struct Type final {
    inline constexpr static auto const phase = p;

    using Meta = this_::Meta<phase>;
    using Value = ::std::decay_t<T>;

    static_assert(::std::is_same_v<T, Value>);

    Meta meta;
    Value value;
};

template <parent_::parent_::Phase p> struct Type<p, void> final {
    inline constexpr static auto const phase = p;

    using Meta = this_::Meta<phase>;

    Meta meta;
};

template <class T> struct IsType final: ::std::false_type {};

template <parent_::parent_::Phase p, class T>
struct IsType<this_::Type<p, T>> final: ::std::true_type {};

template <class T> inline consteval static auto is_type() noexcept(true) {
    return this_::IsType<T>::value;
}

} // namespace result

template <parent_::Phase p, class T = void> using Result = result::Type<p, T>;

template <class T> inline consteval auto is_result() noexcept(true) {
    return this_::result::is_type<T>();
}

namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

template <class ...> struct ReplaceResult final {};

template <class O, class ... I, class R>
struct ReplaceResult<O(I ...), R> final { using Type = R(I ...); };

} // namespace private_

template <Phase phase, class Target> struct Traits final {
    static_assert(::std::is_class_v<Target>);
    static_assert(parent_::target::is_known<Target>());

    using Input = typename parent_::target::Traits<Target>::Input;
    using Output = typename parent_::target::Traits<Target>::Output;
    using Result = this_::Result<phase, Output>;
    using Prototype = typename this_::private_::ReplaceResult<
        typename parent_::target::Traits<Target>::Prototype, Result
    >::Type;
};

} // namespace action
} // namepsace binding
} // namespace p5::refrigerator::core
