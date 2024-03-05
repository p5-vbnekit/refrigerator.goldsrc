#pragma once

#include <cstdint>

#include <set>
#include <utility>

#include "namespace_.hxx"


namespace p5::refrigerator::features::visibility_controller {
namespace current_pack_ {

namespace parent_ = this_;
namespace this_ = parent_::current_pack_;

struct Type final {
    void on_new_frame() noexcept(false);
    void on_entity_passed() noexcept(false);
    bool on_entity_requested(
        void const *client, void const *payload
    ) noexcept(false);

private:
    struct State_ final {
        bool bad = false;
        ::std::size_t size = 0;
        bool xash = false;
        void const *client = nullptr;
        ::std::set<void const *> clients = {};
        ::std::pair<void const *, void const *> payload = {nullptr, nullptr};
    } state_ = {};
};

} // namespace current_pack_

using CurrentPack_ = current_pack_::Type;

} // namespace p5::refrigerator::features::visibility_controller
