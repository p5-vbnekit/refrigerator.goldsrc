#include <cstdint>

#include <limits>
#include <optional>
#include <iterator>
#include <stdexcept>
#include <algorithm>
#include <type_traits>

#include "fade_trap_.hxx"


namespace p5::refrigerator::features::butchering::fade_trap_ {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

template <class Pointer> inline static auto compare_memory_(
    Pointer const *first, Pointer const *second
) noexcept(true) {
    if (first == second) return true;
    auto const * const first_ = reinterpret_cast<char const *>(first);
    return ::std::equal(
        first_, sizeof(*first) + first_,
        reinterpret_cast<char const *>(second)
    );
}

} // namespace private_

bool this_::Type::state(Pointer pointer) const noexcept(true) {
    if (! pointer) return false;
    auto const iterator_ = attached_.find(pointer);
    if (::std::end(attached_) == iterator_) return false;
    return iterator_->second.caught;
}

void this_::Type::reset() noexcept(true) {
    attached_.clear();
    think_state_.cache = ::std::nullopt;
    think_state_.pointer = nullptr;
}

bool this_::Type::attach(Pointer pointer) noexcept(false) {
    if (! pointer) throw ::std::invalid_argument{"empty pointer"};
    return attached_.insert({pointer, {}}).second;
}

bool this_::Type::detach(Pointer pointer) noexcept(true) {
    return attached_.erase(pointer);
}

void this_::Type::after_think(Pointer pointer) noexcept(false) {
    if (! pointer) throw ::std::invalid_argument{"empty pointer"};
    if (! think_state_.pointer) throw ::std::logic_error{"no current thinker"};
    if (think_state_.pointer != pointer) throw ::std::invalid_argument{
        "locked by another thinker"
    };

    think_state_.pointer = nullptr;
    if (! think_state_.cache) return;

    auto const iterator_ = attached_.find(pointer);
    if (::std::end(attached_) == iterator_) return;

    auto &state_ = iterator_->second;
    auto &cache_ = *(think_state_.cache);

    if ([&] {
        if (! (0 < cache_.v.renderamt)) return false;
        if (! (cache_.v.renderamt > pointer->v.renderamt)) return false;
        if (! (cache_.v.nextthink < pointer->v.nextthink)) return false;
        if (! (1 < (pointer->v.nextthink - cache_.v.nextthink))) return false;
        auto copy_ = cache_;
        copy_.v.renderamt = pointer->v.renderamt;
        copy_.v.nextthink = pointer->v.nextthink;
        return this_::private_::compare_memory_(&copy_, pointer);
    } ()) {
        if (state_.cache) {
            auto const &cache_ = *(state_.cache);
            pointer->v.solid = cache_.solid;
            pointer->v.renderamt = cache_.render_amount;
            pointer->v.rendermode = cache_.render_mode;
            state_.cache = ::std::nullopt;
        }
        else {
            pointer->v.solid = cache_.v.solid;
            pointer->v.renderamt = cache_.v.renderamt;
            pointer->v.rendermode = cache_.v.rendermode;
        }
        pointer->v.nextthink = ::std::numeric_limits<
            ::std::decay_t<decltype(pointer->v.nextthink)>
        >::infinity();
        state_.caught = true;
        dispatcher_.dispatch(pointer);
        return;
    }

    if ([&] {
        if (Solid_::Not != pointer->v.solid) return false;
        if (0 != pointer->v.avelocity.x) return false;
        if (0 != pointer->v.avelocity.y) return false;
        if (0 != pointer->v.avelocity.z) return false;
        if (! (cache_.v.nextthink < pointer->v.nextthink)) return false;
        if (! (1 < (pointer->v.nextthink - cache_.v.nextthink))) return false;
        auto copy_ = cache_;
        if (RenderMode_::Normal == pointer->v.rendermode) {
            if (
                RenderMode_::TransTexture != pointer->v.rendermode
            ) return false;
            if (255 != pointer->v.renderamt) return false;
            copy_.v.renderamt = 255;
            copy_.v.rendermode = RenderMode_::TransTexture;
        }
        copy_.v.solid = Solid_::Not;
        copy_.v.nextthink = pointer->v.nextthink;
        return this_::private_::compare_memory_(&copy_, pointer);
    } ()) state_.cache = ::std::decay_t<decltype(*(state_.cache))>{
        .solid = pointer->v.solid,
        .render_mode = pointer->v.rendermode,
        .render_amount = pointer->v.renderamt
    };

    else state_.cache = ::std::nullopt;
}

void this_::Type::before_think(Pointer pointer) noexcept(false) {
    if (! pointer) throw ::std::invalid_argument{"empty pointer"};
    if (think_state_.pointer) throw ::std::logic_error{"busy"};

    think_state_.cache = ::std::nullopt;
    think_state_.pointer = pointer;

    auto const iterator_ = attached_.find(pointer);
    if (::std::end(attached_) == iterator_) return;
    if (iterator_->second.caught) return;
    think_state_.cache.emplace(*pointer);
}

this_::Subscription this_::Type::on_caught(
    Handler &&handler
) const noexcept(false) {
    if (! handler) throw ::std::invalid_argument{"empty handler"};
    return dispatcher_.subscribe(::std::move(handler));
}

} // namespace p5::refrigerator::features::butchering::fade_trap_
