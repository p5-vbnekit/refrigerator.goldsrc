#pragma once

#include <cstdint>

#include <optional>

#include "namespace_.hxx"

#include "api_.hxx"


namespace p5::refrigerator::features::visibility_controller {
namespace limit_ {

namespace parent_ = this_;
namespace this_ = parent_::limit_;

using Api = parent_::Api_;
using Value = ::std::size_t;

struct Type final {
    using Api = this_::Api;
    using Value = this_::Value;

    Value value() const noexcept(false);

    void on_new_frame(Api const &api) noexcept(false);
    void on_entity_requested(
        bool accepted, void const *client, int index, void const *destination
    ) noexcept(false);

private:
    struct Context_ final {
        struct State final {
            bool xash = false;
            void const *client = nullptr;
            void const *destination = nullptr;
            Value accepted = 0;
            Value requested = 0;
            Value expectation = 0;
        };

        Api const *api = nullptr;
        State state = {};
        Value value = 0;
    };

    ::std::optional<Context_> context_ = ::std::decay_t<decltype(*context_)>{};
};

} // namespace limit_

using Limit_ = limit_::Type;

} // namespace p5::refrigerator::features::visibility_controller
