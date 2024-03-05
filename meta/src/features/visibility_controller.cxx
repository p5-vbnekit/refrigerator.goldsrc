#include <cmath>
#include <cstdint>

#include <map>
#include <limits>
#include <utility>
#include <optional>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <forward_list>

#include <p5/lambda/utils/async/subscription.hxx>

#include "../core.hxx"
#include "../exception.hxx"

#include "visibility_controller.hxx"
#include "visibility_controller/api_.hxx"
#include "visibility_controller/limit_.hxx"
#include "visibility_controller/client_.hxx"
#include "visibility_controller/current_pack_.hxx"


namespace p5::refrigerator::features::visibility_controller {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

} // namespace private_

struct Type::Context_ final {
    using Subscription = ::p5::lambda::utils::async::Subscription;

    struct State_ final {
        struct Limit_ final {
            ::std::size_t hard = 0, soft = 0;
            this_::Limit_ machine = {};
        };

        this_::Api_ api;

        Limit_ limit = {};
        ::std::map<void const *, this_::Client_> clients = {};
        this_::CurrentPack_ current_pack = {};
    };

    bool bad = false;
    ::std::optional<State_> state = ::std::nullopt;

    ::std::forward_list<Subscription> subscriptions = {};

private:
    static void const * const injection_;

    static auto const * inject_() noexcept(true);
};

inline this_::Type::Type::~Type() noexcept(true) = default;

inline this_::Type::Type(
    ::std::decay_t<decltype(context_)> &&context
) noexcept(false):
    context_{::std::move(context)}
{
    if (! context_) throw ::std::invalid_argument{"empty context"};
}

namespace private_ {

namespace root = parent_::parent_::parent_;

inline static auto make_entity_weight(
    auto const &api, auto const &client, auto index
) noexcept(false) {
    constexpr static auto const maximum_ = ::std::numeric_limits<
        double
    >::infinity();
    constexpr static auto const minimum_ = -1 * maximum_;
    if (! (api.clients_limit() <= index)) return minimum_;
    auto const * const pointer_ = api.entity(index);
    if (! pointer_) return maximum_;
    if (::std::decay_t<
        decltype(pointer_->free)
    >::False != pointer_->free) return maximum_;
    auto const &variables_ = pointer_->variables;
    auto const factor_ = [&] () -> ::std::decay_t<decltype(minimum_)> {
        if (::std::decay_t<
            decltype(variables_.dead_state)
        >::Dead <= variables_.dead_state) return 3;
        auto const class_ = [&] () -> ::std::string_view {
            if (0 == variables_.class_name) return {};
            auto const * const pointer_ = reinterpret_cast<
                char const *
            >(api.strings()) + variables_.class_name;
            if (! pointer_) return {};
            return pointer_;
        } ();
        if (class_.empty()) return 0;
        if ("gib" == class_) return 12;
        if ("bodyque" == class_) return 6;
        if ("monster_rat" == class_) return 8;
        if ("monster_cockroach" == class_) return 10;
        if (class_.starts_with("ammo_")) return 1;
        if (class_.starts_with("item_")) return 1;
        if (class_.starts_with("weapon_")) return 1;
        if (class_.starts_with("weaponbox")) return 1;
        return 0;
    } ();
    if (! (0 < factor_)) return minimum_;
    auto [x_, y_, z_] = variables_.origin;
    x_ -= client.x;
    y_ -= client.y;
    z_ -= client.z;
    auto const value_ = ::std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
    return factor_ * ::std::max(
        static_cast<::std::decay_t<decltype(value_)>>(80), value_
    );
}

inline static auto on_game_init(auto &context) noexcept(true) {
    if (context.bad) return;

    auto const &core_ = this_::root::Core::instance();

    try {
        if (context.state) throw ::std::logic_error{"initialized already"};
        context.state.emplace(::std::decay_t<
            decltype(*(context.state))
        >{.api{::std::decay_t<decltype(context.state->api)>{core_.api}}});
    }

    catch (...) {
        context.bad = true;
        context.state = ::std::nullopt;
        core_.log.write<
            ::std::decay_t<decltype(core_.log)>::Message::Level::Error
        >() << "visibility_controller.on_game_init failure: "
        << this_::root::exception::generate_details();
    }
}

inline static auto on_new_frame(auto &context) noexcept(true) {
    if (context.bad) return;

    auto const &core_ = this_::root::Core::instance();

    try {
        if (! context.state) context.state.emplace(::std::decay_t<
            decltype(*(context.state))
        >{.api{::std::decay_t<decltype(context.state->api)>{core_.api}}});
        auto &state_ = *(context.state);

        state_.limit.machine.on_new_frame(state_.api);
        auto const limit_ = state_.limit.machine.value();
        if ((0 < limit_) && (state_.limit.hard < limit_)) {
            state_.limit.hard = limit_;
            state_.limit.soft = limit_ - (limit_ / 16);
            core_.log.write()
            << "visibility limit reached: hard = " << limit_
            << ", soft = " << state_.limit.soft;
        }

        state_.current_pack.on_new_frame();

        if (! state_.clients.empty()) ::std::erase_if(state_.clients, [
            time_ = state_.api.time() - 5
        ] (auto const &item) { return ! (time_ < item.second.times().pack); });
    }

    catch (...) {
        context.bad = true;
        context.state = ::std::nullopt;
        core_.log.write<
            ::std::decay_t<decltype(core_.log)>::Message::Level::Error
        >() << "visibility_controller.on_new_frame failure: "
        << this_::root::exception::generate_details();
    }
}

inline static auto on_add_to_full_pack(
    auto &context, auto const *client,
    auto index, auto const *source, auto const *destination
) noexcept(true) {
    using Result_ = this_::root::core::binding::action::Result<
        this_::root::core::binding::Phase::After, int
    >;

    auto result_ = Result_{.meta = Result_::Meta::Ignored, .value = 1};
    auto const &core_ = this_::root::Core::instance();

    if (! context.bad) try {
        if (! context.state) throw ::std::logic_error{"not initialized"};
        auto &state_ = *(context.state);
        auto const &api_ = state_.api;

        if constexpr (true) {
            auto const * const pointer_ = api_.template result<
                ::std::decay_t<decltype(result_.value)>
            >();
            if (! pointer_) throw ::std::logic_error{
                "unable to determine api result: empty pointer"
            };
            switch (*pointer_) {
            default:
                throw ::std::logic_error{
                    "unable to determine api result: value not in {0, 1}"
                    " (incompatible gamedll patches?)"
                };
                break;
            case 0: break;
            case 1: break;
            }
            result_.value = *pointer_;
        }

        result_.meta = Result_::Meta::Handled;

        if (! client) throw ::std::invalid_argument{
            "empty client pointer"
        };

        if constexpr (true) {
            if (! (0 < index)) throw ::std::invalid_argument{
                "invalid entity index"
            };
            auto const &limit_ = api_.entities_limit();
            if (! (0 < limit_)) throw ::std::logic_error{
                "invalid engine.globals.max_entities"
            };
            if (! (limit_ > index)) throw ::std::invalid_argument{
                "invalid entity index"
            };
            if (! source) throw ::std::invalid_argument{
                "empty entity dictionary pointer"
            };
            if ([&] {
                if (api_.entity(index) == source) return false;
                auto const &free_ = source->free;
                return ::std::decay_t<decltype(free_)>::True != free_;
            } ()) throw ::std::invalid_argument{
                "invalid entity index or dictionary pointer"
            };
        };

        if (! destination) throw ::std::invalid_argument{
            "empty entity state pointer"
        };

        auto const time_ = api_.time();
        auto &pack_ = state_.current_pack;
        auto const accepted_ = 0 != result_.value;

        auto const passed_ = [&] {
            try {
                auto &map_ = state_.clients;
                auto &client_ = map_.insert({client, {}}).first->second;
                if (pack_.on_entity_requested(client, destination)) {
                    auto const &limit_ = state_.limit.soft;
                    if (! (
                        (time_ - 5) < client_.times().overflow
                    )) client_.on_new_pack(time_, limit_);
                    else client_.on_new_pack(time_, limit_, [
                        client, &api_
                    ] (auto index) { return this_::make_entity_weight(
                        api_, client->variables.origin, index
                    ); });
                }
                if (accepted_) return client_.on_entity_accepted(index);
                client_.on_entity_rejected(index);
                return false;
            }
            catch (...) { ::std::throw_with_nested(::std::logic_error{
                "client routine failure"
            }); }
        } ();

        try { state_.limit.machine.on_entity_requested(
            accepted_, client, index, destination
        ); }
        catch (...) { ::std::throw_with_nested(::std::logic_error{
            "limit routine failure"
        }); }

        if (passed_) try { pack_.on_entity_passed(); }
        catch (...) { ::std::throw_with_nested(::std::logic_error{
            "pack routine failure"
        }); }

        else if (accepted_) {
            result_.value = 0;
            result_.meta = Result_::Meta::Override;
        }
    }

    catch (...) {
        context.bad = true;
        context.state = ::std::nullopt;
        core_.log.write<
            ::std::decay_t<decltype(core_.log)>::Message::Level::Error
        >() << "visibility_controller.on_add_to_full_pack failure: "
        << this_::root::exception::generate_details();
    }

    return result_;
}

inline static auto on_game_shutdown(auto &context) noexcept(true) {
    context.bad = false;
    context.state = ::std::nullopt;
}

} // namespace private_

inline auto const * this_::Type::Context_::inject_() noexcept(true) {
    namespace root_ = private_::root;

    auto &core_ = root_::Core::instance();
    using LogLevel_ = ::std::decay_t<decltype(core_.log)>::Message::Level;

    try { core_.inject({.load = [&core_] {
        auto * const bindings_ = core_.bindings();
        if (! bindings_) throw ::std::logic_error{
            "bad core state: empty bindings pointer"
        };

        auto * const container_ = core_.container();
        if (! container_) throw ::std::logic_error{
            "bad core state: empty container pointer"
        };

        auto &&context_ = ::std::decay_t<decltype(this_::Type::context_)>{};
        context_.reset(new ::std::decay_t<decltype(*context_)>);

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::After,
            root_::core::bindings::game::Init
        >([&context_ = *context_] () {
            this_::private_::on_game_init(context_);
        }));

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::After,
            root_::core::bindings::game::StartFrame
        >([&context_ = *context_] () {
            this_::private_::on_new_frame(context_);
        }));

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::After,
            root_::core::bindings::game::AddToFullPack
        >([&context_ = *context_] (
            auto *destination, auto index,
            auto const *source, auto const *client,
            auto && ...
        ) { return this_::private_::on_add_to_full_pack(
            context_, client, index, source, destination
        ); }));

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::Before,
            root_::core::bindings::game::Shutdown
        >([&context_ = *context_] () {
            this_::private_::on_game_shutdown(context_);
        }));

        container_->emplace<this_::Type>(::std::move(context_)).pin();
    }}); } catch (...) {
        core_.invalidate();
        core_.log.write<LogLevel_::Error>()
        << parent_::parent_::exception::generate_details();
    }

    return static_cast<void const *>(nullptr);
}

void const * const this_::Type::Context_::injection_{
    this_::Type::Context_::inject_()
};

} // namespace p5::refrigerator::features::visibility_controller
