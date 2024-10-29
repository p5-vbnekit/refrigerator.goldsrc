#include <set>
#include <tuple>
#include <memory>
#include <utility>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <forward_list>

#include <p5/lambda/utils/event/subscription.hxx>

#include "../binding.hxx"
#include "../bindings.hxx"
#include "../exception.hxx"
#include "../singleton.hxx"

#include "client_variables.hxx"


namespace p5::refrigerator::features::client_variables {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

} // namespace private_

struct Type::Context_ final {
    using Subscription = ::p5::lambda::utils::event::Subscription;

    struct State_ final {
        ::std::set<int> requests = {};
    };

    bool bad = false;
    ::std::optional<State_> state = ::std::nullopt;
    ::std::forward_list<Subscription> bindings = {};

private:
    static void const * const injection_;

    static auto const * inject_() noexcept(true);
};

inline this_::Type::Type::~Type() noexcept(true) = default;

inline this_::Type::Type(::std::shared_ptr<Context_> &&context) noexcept(false):
    context_{::std::move(context)}
{
    if (! context_) throw ::std::invalid_argument{"empty context"};
}

namespace private_ {

namespace root = parent_::parent_::parent_;

inline static auto on_game_init(auto &context) noexcept(true) {
    if (context.bad) return;

    auto const &singleton_ = this_::root::Singleton::instance();

    try {
        if (context.state) throw ::std::logic_error{"initialized already"};
        context.state.emplace(::std::decay_t<decltype(*(context.state))>{});
    }

    catch (...) {
        context.bad = true;
        context.state = ::std::nullopt;
        singleton_.log.write<
            ::std::decay_t<decltype(singleton_.log)>::Message::Level::Error
        >() << "visibility_controller.on_game_init failure: "
        << this_::root::exception::generate_details();
    }
}

inline static auto on_new_frame(auto &context) noexcept(true) {
    if (context.bad) return;

    auto const &singleton_ = this_::root::Singleton::instance();

    try {
        if (! context.state) context.state.emplace(::std::decay_t<
            decltype(*(context.state))
        >{});
        // auto &state_ = *(context.state);
    }

    catch (...) {
        context.bad = true;
        context.state = ::std::nullopt;
        singleton_.log.write<
            ::std::decay_t<decltype(singleton_.log)>::Message::Level::Error
        >() << "visibility_controller.on_new_frame failure: "
        << this_::root::exception::generate_details();
    }
}

inline static auto on_game_shutdown(auto &context) noexcept(true) {
    context.bad = false;
    context.state = ::std::nullopt;
}

} // namespace private_

auto const * this_::Type::Context_::inject_() noexcept(true) {
    namespace root_ = this_::private_::root;

    auto &singleton_ = root_::Singleton::instance();
    using LogLevel_ = ::std::decay_t<decltype(singleton_.log)>::Message::Level;

    auto &&injection_ = [&singleton_] {
        auto &&context_ = [] {
            auto * const pointer_ = new this_::Type::Context_{};
            return ::std::shared_ptr<
                ::std::decay_t<decltype(*pointer_)>
            >{pointer_};
        } ();

        context_->bindings.push_front(singleton_.bindings.inject<
            root_::binding::Phase::After,
            root_::bindings::game::Init
        >([&context_ = *context_] () {
            this_::private_::on_game_init(context_);
        }).take());

        context_->bindings.push_front(singleton_.bindings.inject<
            root_::binding::Phase::After,
            root_::bindings::game::Frame
        >([&context_ = *context_] () {
            this_::private_::on_new_frame(context_);
        }).take());

        context_->bindings.push_front(singleton_.bindings.inject<
            root_::binding::Phase::Before,
            root_::bindings::game::Shutdown
        >([&context_ = *context_] () {
            this_::private_::on_game_shutdown(context_);
        }).take());

        return ::std::make_tuple(::std::move(context_));
    };

    try {
        singleton_.container().inject<this_::Type>(
            ::std::in_place_t{}, ::std::move(injection_)
        );
    } catch(...) {
        singleton_.log.write<LogLevel_::Error>()
        << root_::exception::generate_details();
    }

    return static_cast<void const *>(nullptr);
}

void const * const this_::Type::Context_::injection_{
    this_::Type::Context_::inject_()
};

} // namespace p5::refrigerator::features::client_variables
