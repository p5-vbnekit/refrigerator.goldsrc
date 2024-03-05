#include <cmath>

#include <utility>
#include <optional>
#include <stdexcept>
#include <type_traits>

#include "api_.hxx"


namespace p5::refrigerator::features::visibility_controller::api_ {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

inline static auto time(auto &context) noexcept(false) {
    if (
        context.driver.engine.globals != context.cache.globals.engine
    ) throw ::std::logic_error{"globals.engine pointer changed"};

    auto const &value_ = context.cache.globals.engine->time;
    if (! (
        context.cache.time <= value_ && ::std::isfinite(value_)
    )) throw ::std::logic_error{"bad time value"};

    return context.cache.time = value_;
}

inline static auto strings(auto &context) noexcept(false) {
    if (
        context.driver.engine.globals != context.cache.globals.engine
    ) throw ::std::logic_error{"globals.engine pointer changed"};

    auto const &value_ = context.cache.globals.engine->string_base;
    if (! value_) throw ::std::logic_error{"bad strings value"};
    if (! context.cache.strings) context.cache.strings = value_;
    else if (
        context.cache.strings != value_
    ) throw ::std::logic_error{"strings value changed"};

    return value_;
}

inline static auto clients_limit(auto const &context) noexcept(false) {
    if (
        context.driver.engine.globals != context.cache.globals.engine
    ) throw ::std::logic_error{"globals.engine pointer changed"};

    auto const &value_ = context.cache.globals.engine->max_clients;
    if (! (0 < value_)) throw ::std::logic_error{"clients_limit too small"};
    if (! (33 > value_)) throw ::std::logic_error{
        "clients_limit too large"
    };

    auto &cache_ = context.cache.clients_limit;
    if (0 == cache_) cache_ = value_;
    else if (cache_ != value_) throw ::std::logic_error{
        "clients_limit changed"
    };

    return value_;
}

inline static auto entities_limit(auto const &context) noexcept(false) {
    if (
        context.driver.engine.globals != context.cache.globals.engine
    ) throw ::std::logic_error{"globals.engine pointer changed"};

    auto const &value_ = context.cache.globals.engine->max_entities;
    if (! (256 < value_)) throw ::std::logic_error{"entities_limit too small"};
    if (! ((1204 * 1024) > value_)) throw ::std::logic_error{
        "entities_limit too large"
    };

    auto &cache_ = context.cache.entities_limit;
    if (0 == cache_) cache_ = value_;
    else if (cache_ != value_) throw ::std::logic_error{
        "entities_limit changed"
    };

    return value_;
}

inline static auto entity(auto const &context, auto index) noexcept(false) {
    if (! (0 <= index)) throw ::std::invalid_argument{
        "negative entity index"
    };

    auto const &limit_ = this_::entities_limit(context);
    if (! (limit_ > index)) throw ::std::invalid_argument{
        "invalid entity index"
    };

    auto const &driver_ = context.driver.engine.functions;

    if (
        driver_.entityOfEntityIndex != context.cache.entity
    ) throw ::std::logic_error{"entity function pointer changed"};

    return context.cache.entity(index);
}

inline static auto result(auto const &context) noexcept(false) {
    if (
        context.driver.meta.globals != context.cache.globals.meta
    ) throw ::std::logic_error{"globals.meta pointer changed"};
    auto const * const override_ = context.cache.globals.meta->override;
    return override_ ? override_ : context.cache.globals.meta->original;
}

} // namespace private_

auto this_::Type::time() const noexcept(false) -> ::std::decay_t<
    decltype(context_->cache.time)
> {
    if (! context_) throw ::std::logic_error{"bad state"};
    try { return this_::private_::time(*context_); }
    catch (...) { context_.reset(); throw; }
}

auto this_::Type::strings() const noexcept(false) -> ::std::decay_t<
    decltype(context_->cache.strings)
> {
    if (! context_) throw ::std::logic_error{"bad state"};
    try { return this_::private_::strings(*context_); }
    catch (...) { context_.reset(); throw; }
}


auto this_::Type::entity(int index) const noexcept(false) -> ::std::decay_t<
    decltype(context_->cache.entity(index))
> {
    if (! context_) throw ::std::logic_error{"bad state"};
    try { return this_::private_::entity(*context_, index); }
    catch (...) { context_.reset(); throw; }
}

auto this_::Type::clients_limit() const noexcept(false) -> ::std::decay_t<
    decltype(context_->cache.clients_limit)
> {
    if (! context_) throw ::std::logic_error{"bad state"};
    try { return this_::private_::clients_limit(*context_); }
    catch (...) { context_.reset(); throw; }
}

auto this_::Type::entities_limit() const noexcept(false) -> ::std::decay_t<
    decltype(context_->cache.entities_limit)
> {
    if (! context_) throw ::std::logic_error{"bad state"};
    try { return this_::private_::entities_limit(*context_); }
    catch (...) { context_.reset(); throw; }
}

this_::Type::Type(Type &&other) noexcept(true):
    context_{::std::exchange(other.context_, ::std::nullopt)}
{}

this_::Type::Type(Type const &other) noexcept(true) = default;

this_::Type::Type(Driver const &driver) noexcept(false):
    context_{::std::decay_t<decltype(*context_)>{.driver{driver}}}
{
    try {
        auto &context_ = *(this->context_);

        if (! (
            context_.cache.globals.meta = driver.meta.globals
        )) throw ::std::invalid_argument{"empty meta.globals pointer"};

        if (! (
            context_.cache.globals.engine = driver.engine.globals
        )) throw ::std::invalid_argument{"empty engine.globals pointer"};

        if (! (0 <= (
            context_.cache.time = context_.cache.globals.engine->time)
        )) throw ::std::invalid_argument{"bad time value"};
        if (! ::std::isfinite(
            context_.cache.time
        )) throw ::std::invalid_argument{"bad time value"};

        auto const &engine_functions_ = context_.driver.engine.functions;
        if (! (
            context_.cache.entity = engine_functions_.entityOfEntityIndex
        )) throw ::std::logic_error{"empty entity function pointer"};
    }

    catch (...) { context_.reset(); throw; }
}

void const * this_::Type::result_() const noexcept(false) {
    if (! context_) throw ::std::logic_error{"bad state"};
    try { return this_::private_::result(*context_); }
    catch (...) { context_.reset(); throw; }
}

} // namespace p5::refrigerator::features::visibility_controller::api_
