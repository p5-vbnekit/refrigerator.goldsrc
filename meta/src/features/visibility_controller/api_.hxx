#pragma once

#include <optional>
#include <type_traits>

#include "namespace_.hxx"
#include "../../core/api.hxx"


namespace p5::refrigerator::features::visibility_controller {
namespace api_ {

namespace parent_ = this_;
namespace this_ = parent_::api_;

using Driver = parent_::parent_::parent_::core::Api;

struct Type final {
    using Driver = this_::Driver;

private:
    struct Context_ final {
        Driver const &driver;

        struct Cache final {
            struct Globals final {
                ::std::decay_t<decltype(driver.meta.globals)> meta = nullptr;
                ::std::decay_t<
                    decltype(driver.engine.globals)
                > engine = nullptr;
            } globals = {};

            ::std::decay_t<decltype(globals.engine->time)> time = 0;
            ::std::decay_t<
                decltype(globals.engine->string_base)
            > strings = nullptr;

            ::std::decay_t<decltype(
                driver.engine.functions.entityOfEntityIndex
            )> entity = nullptr;

            ::std::decay_t<decltype(
                globals.engine->max_clients
            )> clients_limit = 0;

            ::std::decay_t<decltype(
                globals.engine->max_entities
            )> entities_limit = 0;
        } mutable cache = {};
    };

    mutable ::std::optional<Context_> context_;

public:
    template <class Value> auto const * result() const noexcept(false);

    auto time() const noexcept(false) -> ::std::decay_t<
        decltype(context_->cache.time)
    >;

    auto strings() const noexcept(false) -> ::std::decay_t<
        decltype(context_->cache.strings)
    >;

    auto entity(int index) const noexcept(false) -> ::std::decay_t<
        decltype(context_->cache.entity(index))
    >;

    auto clients_limit() const noexcept(false) -> ::std::decay_t<
        decltype(context_->cache.clients_limit)
    >;

    auto entities_limit() const noexcept(false) -> ::std::decay_t<
        decltype(context_->cache.entities_limit)
    >;

    Type(Type &&) noexcept(true);
    Type(Type const &) noexcept(true);
    explicit Type(Driver const &driver) noexcept(false);

private:
    void const * result_() const noexcept(false);

    Type() = delete;
    Type & operator = (Type &&) noexcept(true) = delete;
    Type & operator = (Type const &) noexcept(true) = delete;
};

} // namespace api_

using Api_ = api_::Type;

namespace api_ {

template <class Value> inline
auto const * this_::Type::result() const noexcept(false) {
    static_assert(::std::is_same_v<::std::decay_t<Value>, Value>);
    return static_cast<Value const *>(result_());
}

} // namespace api_
} // namespace p5::refrigerator::features::visibility_controller
