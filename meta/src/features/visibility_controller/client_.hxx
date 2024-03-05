#pragma once

#include <cstdint>

#include <map>
#include <list>
#include <limits>
#include <utility>
#include <optional>
#include <iterator>
#include <stdexcept>
#include <functional>
#include <type_traits>

#include "namespace_.hxx"


namespace p5::refrigerator::features::visibility_controller {
namespace client_ {

namespace parent_ = this_;
namespace this_ = parent_::client_;

struct Type final {
    auto const & times() const noexcept(false);

    auto on_new_pack(auto &&time) noexcept(false);
    auto on_new_pack(auto &&time, auto &&limit) noexcept(false);

    auto on_new_pack(
        auto &&time, auto &&limit, auto &&evaluator
    ) noexcept(false);

    bool on_entity_accepted(int index) noexcept(false);
    void on_entity_rejected(int index) noexcept(false);

private:
    struct Context_ final {
        struct Times final {
            double pack = ::std::numeric_limits<
                ::std::decay_t<decltype(pack)>
            >::quiet_NaN();
            double overflow = ::std::numeric_limits<
                ::std::decay_t<decltype(pack)>
            >::quiet_NaN();
        };

        using Processed = ::std::map<int, bool>;

        struct Evaluated final {
            ::std::list<::std::pair<int, double>> first = {};
            ::std::decay_t<decltype(first)> second = {};
            ::std::map<int, ::std::pair<
                bool, ::std::decay_t<decltype(::std::begin(first))
            >>> map = {};
        };

        Times times = {};

        ::std::size_t limit = 0;
        ::std::size_t offset = 0;
        bool overflow = false;

        Processed processed = {};
        Evaluated evaluated = {};

        ::std::function<double(int)> evaluator = {};
    };

    ::std::optional<Context_> context_ = ::std::decay_t<decltype(*context_)>{};

    void on_new_pack_(
        ::std::decay_t<decltype(context_->times.pack)>,
        ::std::decay_t<decltype(context_->limit)>,
        ::std::decay_t<decltype(context_->evaluator)> && = {}
    ) noexcept(false);
};

} // namespace client_

using Client_ = client_::Type;

namespace client_ {

inline auto const & this_::Type::times() const noexcept(false) {
    if (! context_) throw ::std::logic_error{"bad state"};
    return context_->times;
}

inline auto this_::Type::on_new_pack(auto &&time) noexcept(false) {
    on_new_pack(::std::forward<decltype(time)>(time), 0);
}

inline auto this_::Type::on_new_pack(
    auto &&time, auto &&limit
) noexcept(false) {
    if (! context_) throw ::std::logic_error{"bad state"};

    try { on_new_pack_(
        ::std::forward<decltype(time)>(time),
        ::std::forward<decltype(limit)>(limit)
    ); }

    catch (...) { context_.reset(); throw; }
}

inline auto this_::Type::on_new_pack(
    auto &&time, auto &&limit, auto &&evaluator
) noexcept(false) {
    if (! context_) throw ::std::logic_error{"bad state"};

    try {
        if (! (0 < limit)) throw ::std::invalid_argument{"zero limit"};

        auto &&evaluator_ = ::std::decay_t<decltype(context_->evaluator)>{
            ::std::forward<decltype(evaluator)>(evaluator)
        };

        if (! evaluator_) throw ::std::invalid_argument{"empty evaluator"};

        on_new_pack_(
            ::std::forward<decltype(time)>(time),
            ::std::forward<decltype(limit)>(limit),
            ::std::move(evaluator_)
        );
    }

    catch (...) { context_.reset(); throw; }
}

} // namespace client_
} // namespace p5::refrigerator::features::visibility_controller
