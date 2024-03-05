#include <tuple>
#include <memory>
#include <utility>
#include <iterator>
#include <algorithm>
#include <exception>
#include <type_traits>
#include <forward_list>

#include <p5/lambda/utils/event/dispatcher.hxx>

#include "../binding.hxx"
#include "../bindings.hxx"
#include "../exception.hxx"
#include "../singleton.hxx"

#include "trusted_entities.hxx"


namespace p5::refrigerator::features::trusted_entities {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

} // namespace private_

struct Type::Context_ final {
    using List = this_::List;

    struct Event final {
        List list;
        bool state;
    };

    using Dispatcher = ::p5::lambda::utils::event::Dispatcher<Event>;
    using Subscription = this_::Subscription;

    bool state = false;
    List queue = {}, entered = {};
    Dispatcher dispatcher = {};
    ::std::forward_list<Subscription> bindings = {};

private:
    static void const * const injection_;

    static auto const * inject_() noexcept(true);
};

this_::List const & this_::Type::state() const noexcept(true) {
    return context_->entered;
}

this_::Subscription this_::Type::on_exit(Handler &&handler) const noexcept(false) {
    if (! handler) throw ::std::invalid_argument{"empty handler"};
    return context_->dispatcher.subscribe([
        handler_ = ::std::move(handler)
    ] (auto const &event) {
        if (event.state) return;
        handler_(event.list);
    });
}

this_::Subscription this_::Type::on_enter(Handler &&handler) const noexcept(false) {
    if (! handler) throw ::std::invalid_argument{"empty handler"};
    return context_->dispatcher.subscribe([
        handler_ = ::std::move(handler)
    ] (auto const &event) {
        if (event.state) handler_(event.list);
    });
}

inline this_::Type::Type::~Type() noexcept(true) = default;

inline this_::Type::Type(::std::shared_ptr<Context_> &&context) noexcept(false):
    context_{::std::move(context)}
{
    if (! context_) throw ::std::invalid_argument{"empty context"};
}

namespace private_ {

namespace root = parent_::parent_::parent_;

template <class Context> inline static
auto on_init(Context &context) noexcept(true) {
    context.state = true;
}

template <class Context, class Entity> inline static
auto on_spawn(Context &context, Entity *entity) noexcept(false) {
    if (! entity) return;
    auto const end_ = ::std::end(context.queue);
    if (end_ != ::std::find(
        ::std::begin(context.queue), end_, entity)
    ) return;
    context.queue.push_back(entity);
}

template <class Meta, class Context> inline static
auto on_create(Meta const &meta, Context &context) noexcept(false) {
    auto *pointer_ = meta.override;

    if (! pointer_) {
        pointer_ = meta.original;
        if (! pointer_) return;
    }

    using Pointer_ = ::std::decay_t<decltype(context.queue.front())>;
    pointer_ = *static_cast<Pointer_ const *>(pointer_);
    if (pointer_) this_::on_spawn(context, static_cast<Pointer_>(
        const_cast<void *>(pointer_)
    ));
}

template <class Context, class Entity> inline static
auto on_remove(Context &context, Entity *entity) noexcept(true) {
    if (! entity) return;
    context.queue.remove(entity);
    if (! (0 < context.entered.remove(entity))) return;
    context.dispatcher.dispatch(typename ::std::decay_t<
        decltype(context.dispatcher)
    >::Event{.list = {entity}, .state = false});
}

template <class Context> inline static
auto on_frame(Context &context) noexcept(true) {
    if (! context.state) return;

    using Event_ = typename ::std::decay_t<
        decltype(context.dispatcher)
    >::Event;

    using Boolean_ = ::p5::lambda::common::IntegerBoolean;

    auto &&removed_ = [&entered_ = context.entered] {
        auto const begin_ = ::std::remove_if(
            ::std::begin(entered_),
            ::std::end(entered_),
            [] (auto const *entity) { return Boolean_::False != entity->free; }
        );
        auto const end_ = ::std::end(entered_);
        auto result_ = Event_{.list = {}, .state = false};
        result_.list.insert(
            ::std::begin(result_.list),
            ::std::move_iterator<::std::decay_t<decltype(begin_)>>{begin_},
            ::std::move_iterator<::std::decay_t<decltype(end_)>>{end_}
        );
        return result_;
    } ();

    auto &&entered_ = [&context] {
        auto result_ = Event_{
            .list = ::std::move(context.queue), .state = true
        };
        result_.list.remove_if([] (auto const *entity) {
            return Boolean_::False != entity->free;
        });
        return result_;
    } ();

    if (! entered_.list.empty()) context.entered.insert(
        ::std::end(context.entered),
        ::std::begin(entered_.list),
        ::std::end(entered_.list)
    );

    if (! removed_.list.empty()) context.dispatcher.dispatch(
        ::std::move(removed_)
    );

    if (! entered_.list.empty()) context.dispatcher.dispatch(
        ::std::move(entered_)
    );
}

template <class Context> inline static
auto on_shutdown(Context &context) noexcept(true) {
    context.state = false;
    context.queue.clear();
    if (context.entered.empty()) return;
    context.dispatcher.dispatch(typename ::std::decay_t<
        decltype(context.dispatcher)
    >::Event{
        .list = ::std::move(context.entered), .state = false
    });
}

} // namespace private_

auto const * this_::Type::Context_::inject_() noexcept(true) {
    namespace root_ = private_::root;

    auto &singleton_ = root_::Singleton::instance();
    using LogLevel_ = ::std::decay_t<decltype(singleton_.log)>::Message::Level;

    auto &&injection_ = [&singleton_] {
        auto context_ = ::std::make_shared<this_::Type::Context_>();

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            private_::root::binding::Phase::After,
            private_::root::bindings::engine::CreateEntity
        >([
            &globals_ = singleton_.api.meta.globals,
            &context_ = *context_, &log_ = singleton_.log
        ] () {
            if (! globals_) return;
            try { this_::private_::on_create(*globals_, context_); }
            catch (...) {
                log_.write<LogLevel_::Error>()
                << root_::exception::generate_details();
            }
        }).take()));

        singleton_.bindings.inject<
            private_::root::binding::Phase::Before,
            private_::root::bindings::engine::RemoveEntity
        >([&context_ = *context_] (auto const *) {});

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            private_::root::binding::Phase::Before,
            private_::root::bindings::engine::CreateNamedEntity
        >([
            &globals_ = singleton_.api.meta.globals,
            &context_ = *context_, &log_ = singleton_.log
        ] (auto) {
            if (! globals_) return;
            try { this_::private_::on_create(*globals_, context_); }
            catch (...) {
                log_.write<LogLevel_::Error>()
                << root_::exception::generate_details();
            }
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            private_::root::binding::Phase::Before,
            private_::root::bindings::engine::RemoveEntity
        >([&context_ = *context_] (auto *entity) {
            this_::private_::on_remove(context_, entity);
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            private_::root::binding::Phase::After,
            private_::root::bindings::game::Init
        >([&context_ = *context_] () {
            this_::private_::on_init(context_);
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            private_::root::binding::Phase::After,
            private_::root::bindings::game::Spawn
        >([&context_ = *context_, &log_ = singleton_.log] (auto *entity) {
            try { this_::private_::on_spawn(context_, entity); }
            catch (...) {
                log_.write<LogLevel_::Error>()
                << root_::exception::generate_details();
            }
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            private_::root::binding::Phase::Before,
            private_::root::bindings::game::Frame
        >([&context_ = *context_] {
            this_::private_::on_frame(context_);
        }).take()));

        context_->bindings.push_front(::std::move(singleton_.bindings.inject<
            private_::root::binding::Phase::Before,
            private_::root::bindings::game::Shutdown
        >([&context_ = *context_] {
            this_::private_::on_shutdown(context_);
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

} // namespace p5::refrigerator::features::trusted_entities
