#include <cmath>

#include <list>
#include <utility>
#include <iterator>
#include <stdexcept>
#include <type_traits>

#include "client_.hxx"


namespace p5::refrigerator::features::visibility_controller::client_ {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

inline static auto on_entity_passed_(auto &context) noexcept(true) {
    if (! (0 < context.limit)) return true;
    if (context.limit > ++(context.offset)) return true;
    context.overflow = true;
    context.evaluator = nullptr;
    context.evaluated.map.clear();
    context.evaluated.first.clear();
    context.evaluated.second.clear();
    context.times.overflow = context.times.pack;
    return false;
}

inline static auto on_entity_accepted(
    auto &context, auto index
) noexcept(false) {
    if (
        ! context.processed.insert({index, true}).second
    ) throw ::std::invalid_argument{"entity processed already"};

    if (context.overflow) return false;

    if (! context.evaluator) return on_entity_passed_(context);

    if (! (
        (0 < context.limit) && (context.limit > context.offset)
    )) throw ::std::logic_error{"bad state"};

    auto const node_ = context.evaluated.map.extract(index);

    if (node_) {
        auto iterator_ = node_.mapped().second;

        if (node_.mapped().first) {
            context.evaluated.first.erase(iterator_);
            return on_entity_passed_(context);
        }

        context.evaluated.second.erase(iterator_);
        return false;
    }

    auto const weight_ = context.evaluator(index);
    if (! ((0 >= weight_) || (0 < weight_))) return false;

    if (
        context.evaluated.first.size() < (context.limit - context.offset)
    ) {
        if (context.evaluated.second.empty()) return on_entity_passed_(context);
        throw ::std::logic_error{"bad state"};
    }

    auto iterator_ = ::std::begin(context.evaluated.first);
    if (! (iterator_->second > weight_)) return false;

    context.evaluated.second.splice(
        ::std::begin(context.evaluated.second),
        context.evaluated.first, iterator_
    );
    iterator_ = ::std::begin(context.evaluated.second);
    context.evaluated.map[iterator_->first] = {false, iterator_};
    return on_entity_passed_(context);
}

inline static auto on_entity_rejected(
    auto &context, auto index
) noexcept(false) {
    if (
        ! context.processed.insert({index, false}).second
    ) throw ::std::invalid_argument{"entity processed already"};

    if (context.overflow) return;
    if (! context.evaluator) return;

    auto const node_ = context.evaluated.map.extract(index);
    if (! node_) return;

    if (! node_.mapped().first) {
        context.evaluated.second.erase(node_.mapped().second);
        return;
    }

    context.evaluated.first.erase(node_.mapped().second);
    if (context.evaluated.second.empty()) return;

    context.evaluated.first.splice(
        ::std::begin(context.evaluated.first),
        context.evaluated.second, ::std::begin(context.evaluated.second)
    );

    auto const iterator_ = ::std::begin(context.evaluated.first);
    context.evaluated.map[iterator_->first] = {true, iterator_};
}

inline static auto on_new_pack(
    auto &context, auto &&time, auto &&limit, auto &&evaluator
) noexcept(false) {
    if (! ::std::isfinite(time)) throw ::std::invalid_argument{"bad time"};
    if (::std::isfinite(context.times.pack) && (! (
        context.times.pack <= time
    ))) throw ::std::invalid_argument{"bad time"};

    context.limit = ::std::forward<decltype(limit)>(limit);
    context.evaluator = ::std::forward<decltype(evaluator)>(evaluator);
    if (
        context.evaluator && (! (0 < context.limit))
    ) throw ::std::invalid_argument{"bad limit"};
    context.times.pack = ::std::forward<decltype(time)>(time);

    context.offset = 0;
    context.overflow = false;
    context.evaluated.map.clear();
    context.evaluated.first.clear();
    context.evaluated.second.clear();

    if (context.processed.empty()) return;

    if (! context.evaluator) {
        context.processed.clear();
        return;
    }

    auto list_ = ::std::decay_t<decltype(context.evaluated.first)>{};

    for (auto const &processed_: context.processed) {
        if (! processed_.second) continue;
        auto const weight_ = context.evaluator(processed_.first);
        if (! ((0 >= weight_) || (0 < weight_))) continue;
        list_.push_back({processed_.first, weight_});
    }

    context.processed.clear();
    if (list_.empty()) return;

    list_.sort([] (
        auto const &l, auto const &r
    ) { return l.second < r.second; });

    auto offset_ = static_cast<::std::decay_t<decltype(context.offset)>>(0);

    do {
        if (context.limit > offset_) {
            ++offset_;
            context.evaluated.first.splice(
                ::std::begin(context.evaluated.first),
                list_, ::std::begin(list_)
            );
            auto const iterator_ = ::std::begin(context.evaluated.first);
            context.evaluated.map.insert({iterator_->first, {true, iterator_}});
            continue;
        }
        context.evaluated.second.splice(
            ::std::end(context.evaluated.second),
            list_, ::std::begin(list_)
        );
        auto const iterator_ = ::std::prev(::std::end(
            context.evaluated.second
        ));
        context.evaluated.map.insert({iterator_->first, {false, iterator_}});
    } while (! list_.empty());
}

} // namespace private_

bool this_::Type::on_entity_accepted(int index) noexcept(false) {
    if (! context_) throw ::std::logic_error{"bad state"};
    try { return this_::private_::on_entity_accepted(*context_, index); }
    catch (...) { context_.reset(); throw; }
}

void this_::Type::on_entity_rejected(int index) noexcept(false) {
    if (! context_) throw ::std::logic_error{"bad state"};
    try { return this_::private_::on_entity_rejected(*context_, index); }
    catch (...) { context_.reset(); throw; }
}

void this_::Type::on_new_pack_(
    ::std::decay_t<decltype(context_->times.pack)> time,
    ::std::decay_t<decltype(context_->limit)> limit,
    ::std::decay_t<decltype(context_->evaluator)> &&evaluator
) noexcept(false) {
    this_::private_::on_new_pack(
        *context_, time, limit, ::std::move(evaluator)
    );
}

} // namespace p5::refrigerator::features::visibility_controller::client_
