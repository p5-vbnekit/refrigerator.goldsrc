#include <cstdint>

#include <utility>
#include <stdexcept>

#include "current_pack_.hxx"


namespace p5::refrigerator::features::visibility_controller::current_pack_ {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

inline static auto distance(void const *begin, void const *end) noexcept(true) {
    return static_cast<::std::size_t>(
        static_cast<char const *>(end) - static_cast<char const *>(begin)
    );
}

} // namespace private_

void this_::Type::on_new_frame() noexcept(false) {
    if (state_.bad) throw ::std::logic_error{"bad state"};
    state_ = {.size = state_.size};
}

void this_::Type::on_entity_passed() noexcept(false) {
    if (state_.bad) throw ::std::logic_error{"bad state"};
    if (! state_.payload.second) throw ::std::logic_error{
        "not in request state"
    };
    state_.payload.first = ::std::exchange(state_.payload.second, nullptr);
}

bool this_::Type::on_entity_requested(
    void const *client, void const *payload
) noexcept(false) {
    if (state_.bad) throw ::std::logic_error{"bad state"};

    try {
        if (! client) throw ::std::invalid_argument{"empty client pointer"};
        if (! payload) throw ::std::invalid_argument{"empty payload pointer"};

        if (state_.client != client) {
            auto const is_new_client_ = state_.clients.insert(client).second;
            if (! is_new_client_) throw ::std::invalid_argument{
                "client processed already"
            };
            state_.client = client;
            state_.payload = {nullptr, payload};
            return true;
        }

        if (auto const payload_ = state_.payload.second) {
            if (payload_ != payload) throw ::std::invalid_argument{
                "invalid payload pointer"
            };
            return false;
        }

        if (auto const payload_ = state_.payload.first) {
            if (0 == state_.size) {
                if (! (payload_ < payload)) throw ::std::invalid_argument{
                    "invalid payload pointer"
                };
                state_.size = this_::private_::distance(payload_, payload);
            }
            else if (state_.xash) {
                if (payload_ != payload) throw ::std::invalid_argument{
                    "invalid payload pointer"
                };
            }
            else {
                auto const distance_ = this_::private_::distance(
                    payload_, payload
                );
                if (0 == distance_) state_.xash = true;
                else if (
                    state_.size != distance_
                ) throw ::std::invalid_argument{"invalid payload pointer"};
            }
        }

        state_.payload.second = payload;
        return false;
    }

    catch (...) { state_ = {.bad = true}; throw; }
}

} // namespace p5::refrigerator::features::visibility_controller::current_pack_
