#include <tuple>
#include <memory>
#include <utility>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <forward_list>

#include <p5/lambda/utils/container/pointer.hxx>
#include <p5/lambda/utils/event/subscription.hxx>

#include "../api.hxx"
#include "../core.hxx"
#include "../binding.hxx"
#include "../bindings.hxx"
#include "../exception.hxx"

#include "deity.hxx"
#include "trusted_entities.hxx"


namespace p5::refrigerator::features::deity {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

} // namespace private_

struct Type::Context_ final {
    using Engine = parent_::parent_::api::Engine;
    using Subscription = ::p5::lambda::utils::event::Subscription;
    using EntityPointer = this_::EntityPointer;
    using TrustedPointer = ::p5::lambda::utils::container::Pointer<
        parent_::TrustedEntities
    >;

    bool state = false;
    Engine const &engine;
    TrustedPointer const trusted;

    ::std::forward_list<Subscription> bindings = {};

private:
    static void const * const injection_;

    static auto const * inject_() noexcept(true);
};

this_::EntityPointer this_::Type::create_entity(
    char const *class_name
) noexcept(false) {
    if (! context_->state) throw ::std::logic_error{"invalid state"};
    auto const &engine_ = context_->engine;

    if (
        context_->trusted && (! engine_.functions.removeEntity)
    ) throw ::std::logic_error{"`engine.remove_entity` is null"};

    auto const class_ = [&class_name] () -> ::std::string_view {
        if (class_name) return class_name;
        return {};
    } ();

    auto *pointer_ = static_cast<EntityPointer>(nullptr);

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
            if (! globals_->pStringBase) throw ::std::logic_error{
                "`engine.globals.string_base` is null"
            };
            return static_cast<int>(class_ - globals_->pStringBase);
        } ());

        if (! pointer_) throw ::std::logic_error{
            "null pointer returned from `engine.create_named_entity`"
        };
    }

    if (context_->trusted) {
        try { context_->trusted->insert(pointer_); }
        catch (...) {
            engine_.functions.removeEntity(pointer_);
            throw;
        }
    }

    return pointer_;
}

void this_::Type::remove_entity(EntityPointer pointer) noexcept(false) {
    if (! pointer) throw ::std::invalid_argument{"empty pointer"};

    if (! context_->engine.functions.removeEntity) throw ::std::logic_error{
        "`engine.remove_entity` is null"
    };

    if (context_->trusted) context_->trusted->remove(pointer);
    context_->engine.functions.removeEntity(pointer);
}

inline this_::Type::Type::~Type() noexcept(true) = default;

inline this_::Type::Type(::std::shared_ptr<Context_> &&context) noexcept(false):
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

auto const * this_::Type::Context_::inject_() noexcept(true) {
    namespace root_ = this_::private_::root;

    auto &singleton_ = root_::Singleton::instance();
    using LogLevel_ = ::std::decay_t<decltype(singleton_.log)>::Message::Level;

    auto &&injection_ = [&singleton_] {
        auto &&context_ = [&singleton_] {
            auto const &engine_ = singleton_.api.engine;
            auto &&trusted_ = singleton_.container().get<
                parent_::TrustedEntities
            >(::std::rethrow_exception);
            auto * const pointer_ = new this_::Type::Context_{
                .engine = engine_, .trusted = ::std::move(trusted_)
            };
            return ::std::shared_ptr<
                ::std::decay_t<decltype(*pointer_)>
            >{pointer_};
        } ();

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            root_::binding::Phase::After,
            root_::bindings::game::Init
        >([&context_ = *context_] () {
            this_::private_::on_init(context_);
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            root_::binding::Phase::Before,
            root_::bindings::game::Frame
        >([&context_ = *context_] {
            this_::private_::on_frame(context_);
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            root_::binding::Phase::Before,
            root_::bindings::game::Shutdown
        >([&context_ = *context_] {
            this_::private_::on_shutdown(context_);
        }).take()));

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

} // namespace p5::refrigerator::features::deity
