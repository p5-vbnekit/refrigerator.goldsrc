#include <memory>
#include <utility>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <forward_list>

#include <p5/lambda/utils/async/subscription.hxx>

#include "../core.hxx"
#include "../exception.hxx"

#include "deity.hxx"
#include "trusted_entities.hxx"


namespace p5::refrigerator::features::deity {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

} // namespace private_

struct Type::Context_ final {
    using Entity = this_::Entity;
    using Engine = parent_::parent_::core::api::Engine;
    using Trusted = parent_::TrustedEntities;
    using Subscription = ::p5::lambda::utils::async::Subscription;

    bool state = false;
    Engine const &engine;
    Trusted &trusted;

    ::std::forward_list<Subscription> subscriptions = {};

private:
    static void const * const injection_;

    static auto const * inject_() noexcept(true);
};

this_::Entity * this_::Type::create_entity(
    char const *class_name
) noexcept(false) {
    if (! context_->state) throw ::std::logic_error{"invalid state"};
    auto const &engine_ = context_->engine;

    if (! engine_.functions.removeEntity) throw ::std::logic_error{
        "`engine.remove_entity` is null"
    };

    auto const class_ = [&class_name] () -> ::std::string_view {
        if (class_name) return class_name;
        return {};
    } ();

    auto *pointer_ = static_cast<Entity *>(nullptr);

    if (class_.empty()) {
        if (! engine_.functions.createEntity) throw ::std::logic_error{
            "`engine.create_entity` is null"
        };
        pointer_ = engine_.functions.createEntity();
        if (! pointer_) throw ::std::logic_error{
            "null pointer returned from `engine.create_entity`"
        };
    }

    else {
        if (! engine_.functions.createNamedEntity) throw ::std::logic_error{
            "`engine.create_entity` is null"
        };

        pointer_ = engine_.functions.createNamedEntity([
            &globals_ = engine_.globals, class_ = class_.data()
        ] {
            if (! globals_) throw ::std::logic_error{
                "`engine.globals` is null"
            };
            if (! globals_->string_base) throw ::std::logic_error{
                "`engine.globals.string_base` is null"
            };
            return static_cast<int>(class_ - globals_->string_base);
        } ());

        if (! pointer_) throw ::std::logic_error{
            "null pointer returned from `engine.create_named_entity`"
        };
    }

    try { context_->trusted.insert(pointer_); }
    catch (...) {
        engine_.functions.removeEntity(pointer_);
        throw;
    }

    return pointer_;
}

void this_::Type::remove_entity(Entity const *pointer) noexcept(false) {
    if (! pointer) throw ::std::invalid_argument{"empty pointer"};

    if (! context_->engine.functions.removeEntity) throw ::std::logic_error{
        "`engine.remove_entity` is null"
    };

    auto * const pointer_ = const_cast<
        ::std::decay_t<decltype(*pointer)> *
    >(pointer);

    context_->trusted.remove(pointer_);
    context_->engine.functions.removeEntity(pointer_);
}

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

inline static auto on_init(auto &context) noexcept(true) {
    context.state = false;
}

inline static auto on_frame(auto &context) noexcept(true) {
    context.state = true;
}

inline static auto on_shutdown(auto &context) noexcept(true) {
    context.state = false;
}

} // namespace private_

inline auto const * this_::Type::Context_::inject_() noexcept(true) {
    namespace root_ = private_::root;

    auto &core_ = root_::Core::instance();
    using LogLevel_ = ::std::decay_t<decltype(core_.log)>::Message::Level;

    try { core_.inject({.load = [&core_] () -> ::std::decay_t<
        decltype(core_)
    >::Module::Loader::Task {
        auto * const bindings_ = core_.bindings();
        if (! bindings_) throw ::std::logic_error{
            "bad core state: empty bindings pointer"
        };

        auto * const container_ = core_.container();
        if (! container_) throw ::std::logic_error{
            "bad core state: empty container pointer"
        };

        auto &&context_ = ::std::decay_t<decltype(this_::Type::context_)>{};

        auto &trusted_ = co_await core_.container()->get<
            ::std::remove_reference_t<decltype(context_->trusted)>
        >();

        context_.reset(new ::std::decay_t<decltype(*context_)>{
            .engine = core_.api.engine, .trusted = trusted_
        });

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::After,
            root_::core::bindings::game::Init
        >([&context_ = *context_] () {
            this_::private_::on_init(context_);
        }));

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::Before,
            root_::core::bindings::game::StartFrame
        >([&context_ = *context_] {
            this_::private_::on_frame(context_);
        }));

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::Before,
            root_::core::bindings::game::Shutdown
        >([&context_ = *context_] {
            this_::private_::on_shutdown(context_);
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

} // namespace p5::refrigerator::features::deity
