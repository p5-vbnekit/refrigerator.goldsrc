#include <memory>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <forward_list>
#include <unordered_set>
#include <unordered_map>

#include <p5/lambda/common.hxx>
#include <p5/lambda/utils/container/pointer.hxx>
#include <p5/lambda/utils/event/subscription.hxx>

#include "../binding.hxx"
#include "../bindings.hxx"
#include "../exception.hxx"
#include "../singleton.hxx"

#include "butchering.hxx"
#include "junk_controller.hxx"
#include "trusted_entities.hxx"

#include "butchering/fade_trap_.hxx"


namespace p5::refrigerator::features::butchering {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

} // namespace private_

struct Type::Context_ final {
    using Junk = ::p5::lambda::utils::container::Pointer<
        parent_::JunkController
    >;
    using Entity = ::p5::lambda::common::entity::Dictionary;
    using FadeTrap = this_::FadeTrap_;
    using Subscription = ::p5::lambda::utils::event::Subscription;

    Junk junk;
    FadeTrap fade_trap = {};

    ::std::unordered_set<Entity *> gibses = {};
    ::std::unordered_set<Entity *> monsters = {};
    ::std::unordered_set<Entity *> monstermakers = {};

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

inline static auto reset_context(auto &context) noexcept(true) {
    context.fade_trap.reset();
    context.gibses.clear();
    context.monsters.clear();
    context.monstermakers.clear();
}

inline static auto on_fading_out(
    auto const &context, auto *entity
) noexcept(false) {
    if (context.junk) context.junk->assign(entity);
}

inline static auto on_leave_trusted(
    auto &context, auto const &entities
) noexcept(true) {
    for (auto * const entity_: entities) {
        context.fade_trap.detach(entity_);
        context.gibses.erase(entity_);
        context.monsters.erase(entity_);
        context.monstermakers.erase(entity_);
    }
}

inline static auto on_enter_trusted(
    auto const &engine, auto &context, auto const &entities
) noexcept(false) {
    if (! engine.functions.szFromIndex) return;
    for (auto * const entity_: entities) {
        auto const class_ = [
            &resolver_ = engine.functions.szFromIndex,
            &index_ = entity_->v.classname
        ] () -> ::std::string_view {
            if (! resolver_) return {};
            auto const * const pointer_ = resolver_(index_);
            if (! pointer_) return {};
            return pointer_;
        } ();
        if (class_.empty()) continue;
        if ("gib" == class_) {
            context.fade_trap.attach(entity_);
            context.gibses.insert(entity_);
        }
        else if ("monstermaker" == class_) {
            context.monstermakers.insert(entity_);
        }
        else if (class_.starts_with("monster_")) {
            context.fade_trap.attach(entity_);
            context.monsters.insert(entity_);
        }
    }
}

template <this_::root::binding::Phase phase> inline static
auto on_think(auto &context, auto *entity) noexcept(false) {
    if (! entity) throw ::std::invalid_argument{"think: empty entity pointer"};

    if constexpr (::std::decay_t<decltype(phase)>::Before == phase) {
        context.fade_trap.before_think(entity);
    }

    else context.fade_trap.after_think(entity);
}

} // namespace private_

inline auto const * this_::Type::Context_::inject_() noexcept(true) {
    namespace root_ = private_::root;

    auto &singleton_ = root_::Singleton::instance();
    using LogLevel_ = ::std::decay_t<decltype(singleton_.log)>::Message::Level;

    auto &&injection_ = [&singleton_] {
        auto const &container_ = singleton_.container();

        auto const &trusted_ = singleton_.container().get<
            parent_::TrustedEntities const
        >(::std::rethrow_exception).resolve();

        auto &&context_ = [&container_] {
            auto &&junk_ = container_.get<
                parent_::JunkController
            >(::std::rethrow_exception);
            auto * const pointer_ = new this_::Type::Context_{
                .junk = ::std::move(junk_)
            };
            return ::std::shared_ptr<
                ::std::decay_t<decltype(*pointer_)>
            >{pointer_};
        } ();

        context_->fade_trap.on_caught([&context_ = *context_] (auto *entity) {
            this_::private_::on_fading_out(context_, entity);
        });

        context_->bindings.push_front(::std::move(trusted_.on_exit([
            &context_ = *context_
        ] (auto const &entities) {
            this_::private_::on_leave_trusted(context_, entities);
        }).take()));

        context_->bindings.push_front(::std::move(trusted_.on_enter([
            &engine_ = singleton_.api.engine,
            &context_ = *context_, &log_ = singleton_.log
        ] (auto const &entities) {
            try {
                this_::private_::on_enter_trusted(engine_, context_, entities);
            }
            catch (...) {
                log_.write<LogLevel_::Error>()
                << root_::exception::generate_details();
            }
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            root_::binding::Phase::After,
            root_::bindings::game::Init
        >([&context_ = *context_] () {
            this_::private_::reset_context(context_);
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            root_::binding::Phase::Before,
            root_::bindings::game::Think
        >([&singleton_, &context_ = *context_] (auto *entity) {
            try {
                this_::private_::on_think<
                    root_::binding::Phase::Before
                >(context_, entity);
            }
            catch (...) {
                singleton_.log.write<LogLevel_::Error>()
                << root_::exception::generate_details();
            }
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            root_::binding::Phase::After,
            root_::bindings::game::Think
        >([&singleton_, &context_ = *context_] (auto *entity) {
            try {
                this_::private_::on_think<
                    root_::binding::Phase::After
                >(context_, entity);
            }
            catch (...) {
                singleton_.log.write<LogLevel_::Error>()
                << root_::exception::generate_details();
            }
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            root_::binding::Phase::Before,
            root_::bindings::game::Shutdown
        >([&context_ = *context_] () {
            this_::private_::reset_context(context_);
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

} // namespace p5::refrigerator::features::butchering
