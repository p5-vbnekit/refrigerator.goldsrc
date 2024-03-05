#include <cmath>
#include <cstdint>

#include <set>
#include <limits>
#include <memory>
#include <exception>
#include <string_view>
#include <type_traits>
#include <forward_list>

#include <p5/lambda.hxx>

#include "../binding.hxx"
#include "../bindings.hxx"
#include "../exception.hxx"
#include "../singleton.hxx"

#include "butchering.hxx"
#include "trusted_entities.hxx"


namespace p5::refrigerator::features::butchering {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

} // namespace private_

struct Type::Context_ final {
    using Entity = ::p5::lambda::common::entity::Dictionary;
    using Subscription = ::p5::lambda::utils::event::Subscription;

    ::std::set<Entity *> gibses = {};
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

template <class Engine> inline static
auto nearest_think_time_(Engine const &engine) noexcept(true) {
    auto const value_ = [time_ = engine.functions.time] () -> ::std::decay_t<
        decltype(::std::declval<Engine>().functions.time())
    > {
        if (! time_) return 0;
        auto const value_ = time_();
        if (! ((0 < value_) && ::std::isfinite(value_))) return 0;
        return value_;
    } ();
    return ::std::nextafter(value_, 1 + value_);
}

template <class Engine> inline static
::std::size_t fadeout_freezer_capacity_(Engine const &engine) noexcept(true) {
    if (! engine.globals) return 0;
    if (! engine.functions.numberOfEntities) return 0;
    auto const maximum_ = engine.globals->maxEntities;
    if (! (0 < maximum_)) return 0;
    auto const reserved_ = ::std::max(128, maximum_ / 4);
    if (! (reserved_ < maximum_)) return 0;
    auto const limit_ = maximum_ - reserved_;
    auto const total_ = engine.functions.numberOfEntities();
    return (limit_ > total_) ? static_cast<::std::size_t>(limit_ - total_) : 0;
}

template <class Context, class Entities> inline static
auto on_exit(Context &context, Entities const &entities) noexcept(true) {
    for (auto * const entity_: entities) context.gibses.erase(entity_);
}

template <class Engine, class Context, class Entities> inline static
auto on_enter(
    Engine const &engine, Context &context, Entities const &entities
) noexcept(false) {
    if (! engine.functions.szFromIndex) return;
    for (auto * const entity_: entities) {
        auto const class_ = [
            &engine_ = engine.functions, &index_ = entity_->v.classname
        ] () -> ::std::string_view {
            auto const * const pointer_ = engine_.szFromIndex(index_);
            if (! pointer_) return {};
            return pointer_;
        } ();
        if (class_.empty()) continue;
        if ("gib" == class_) context.gibses.insert(entity_);
    }
}

template <class Engine, class Context, class Entity> inline static
auto on_think(
    Engine const &engine, Context &context, Entity *entity
) noexcept(true) {
    using Result_ = private_::root::binding::action::result::Meta<
        private_::root::binding::Phase::Before
    >;

    if (! entity) return Result_::Ignored;
    auto &v_ = entity->v;

    if (context.gibses.contains(entity)) {
        if (! (
            0 < this_::fadeout_freezer_capacity_(engine)
        )) return Result_::Handled;
        using Solid_ = ::std::decay_t<decltype(v_.solid)>;
        if (Solid_::BBox > v_.solid) v_.solid = Solid_::BBox;
        if (255 > v_.renderamt) v_.renderamt = 255;
        v_.nextthink = ::std::numeric_limits<
            ::std::decay_t<decltype(v_.nextthink)>
        >::infinity();
        return Result_::Supercede;
    }

    return Result_::Ignored;
}

template <class Engine, class Context> inline static
auto on_frame(Engine const &engine, Context &context) noexcept(true) {
    if (0 < this_::fadeout_freezer_capacity_(engine)) return;
    if (context.gibses.empty()) return;
    auto &time_ = (**(context.gibses.begin())).v.nextthink;
    if (::std::isfinite(time_)) return;
    time_ = this_::nearest_think_time_(engine);
}

} // namespace private_

inline auto const * this_::Type::Context_::inject_() noexcept(true) {
    namespace root_ = private_::root;

    auto &singleton_ = root_::Singleton::instance();
    using LogLevel_ = ::std::decay_t<decltype(singleton_.log)>::Message::Level;

    auto &&injection_ = [&singleton_] {
        auto context_ = ::std::make_shared<this_::Type::Context_>();

        auto &trusted_ = singleton_.container().get<
            parent_::TrustedEntities
        >(::std::rethrow_exception).resolve();

        context_->bindings.push_front(::std::move(trusted_.on_exit([
            &context_ = *context_, &log_ = singleton_.log
        ] (auto const &entities) {
            try { this_::private_::on_exit(context_, entities); }
            catch (...) {
                log_.write<LogLevel_::Error>()
                << root_::exception::generate_details();
            }
        }).take()));

        context_->bindings.push_front(::std::move(trusted_.on_enter([
            &engine_ = singleton_.api.engine,
            &context_ = *context_, &log_ = singleton_.log
        ] (auto const &entities) {
            try { this_::private_::on_enter(engine_, context_, entities); }
            catch (...) {
                log_.write<LogLevel_::Error>()
                << root_::exception::generate_details();
            }
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            private_::root::binding::Phase::Before,
            private_::root::bindings::game::Think
        >([
            &engine_ = singleton_.api.engine, &context_ = *context_
        ] (auto *entity) {
            return this_::private_::on_think(engine_, context_, entity);
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            private_::root::binding::Phase::After,
            private_::root::bindings::game::Frame
        >([&engine_ = singleton_.api.engine, &context_ = *context_] {
            this_::private_::on_frame(engine_, context_);
        }).take()));

        return ::std::make_tuple(::std::move(context_));
    };

    try {
        auto &container_ = singleton_.container();
        container_.inject<this_::Type>(
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
