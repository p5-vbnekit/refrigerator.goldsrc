#include <utility>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <forward_list>

#include <p5/lambda/common.hxx>
#include <p5/lambda/utils/async/channel/issuer.hxx>

#include "../core.hxx"
#include "../exception.hxx"

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

    using Channel = ::p5::lambda::utils::async::channel::Issuer<Event>;
    using Subscription = this_::Subscription;

    bool state = false;
    List queue = {}, entered = {};
    Channel channel = {};

    ::std::forward_list<Subscription> subscriptions = {};

private:
    static void const * const injection_;

    static auto const * inject_() noexcept(true);
};

this_::List const & this_::Type::queue() const noexcept(true) {
    return context_->queue;
}

this_::List const & this_::Type::entered() const noexcept(true) {
    return context_->entered;
}

bool this_::Type::insert(Item item) noexcept(false) {
    if (! context_->state) throw ::std::logic_error{"invalid state"};
    if (::std::end(context_->entered) != ::std::find(
        ::std::begin(context_->entered),
        ::std::end(context_->entered), item
    )) return false;
    if (::std::end(context_->queue) != ::std::find(
        ::std::begin(context_->queue),
        ::std::end(context_->queue), item
    )) return false;
    context_->queue.push_back(item);
    return true;
}

bool this_::Type::remove(Item item) noexcept(false) {
    if (! context_->state) throw ::std::logic_error{"invalid state"};
    if (0 < context_->queue.remove(item)) return true;
    if (! (0 < context_->entered.remove(item))) return false;
    context_->channel.dispatch(typename ::std::decay_t<
        decltype(context_->channel)
    >::Event{.list = {item}, .state = false});
    return true;
}

this_::Subscription this_::Type::on_exit(
    Handler &&handler
) const noexcept(false) {
    if (! handler) throw ::std::invalid_argument{"empty handler"};
    return context_->channel.subscribe([
        handler_ = ::std::move(handler)
    ] (auto const &event) {
        if (! event.state) handler_(event.list);
    });
}

this_::Subscription this_::Type::on_enter(
    Handler &&handler
) const noexcept(false) {
    if (! handler) throw ::std::invalid_argument{"empty handler"};
    return context_->channel.subscribe([
        handler_ = ::std::move(handler)
    ] (auto const &event) {
        if (event.state) handler_(event.list);
    });
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
    context.queue.clear();
    context.entered.clear();
}

inline static auto on_spawn(auto &context, auto *entity) noexcept(false) {
    if (! entity) return;
    if (::std::end(context.entered) != ::std::find(
        ::std::begin(context.entered), ::std::end(context.entered), entity)
    ) return;
    if (::std::end(context.queue) != ::std::find(
        ::std::begin(context.queue), ::std::end(context.queue), entity)
    ) return;
    context.queue.push_back(entity);
}

inline static auto on_create(auto const &meta, auto &context) noexcept(false) {
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

inline static auto on_remove(auto &context, auto *entity) noexcept(true) {
    if (! entity) return;
    context.queue.remove(entity);
    if (! (0 < context.entered.remove(entity))) return;
    context.channel.dispatch(typename ::std::decay_t<
        decltype(context.channel)
    >::Event{.list = {entity}, .state = false});
}

inline static auto on_frame(auto &context) noexcept(true) {
    context.state = true;

    using Event_ = typename ::std::decay_t<
        decltype(context.channel)
    >::Event;

    using Boolean_ = ::p5::lambda::common::IntegerBoolean;

    auto &&removed_ = [&entered_ = context.entered] {
        auto const begin_ = ::std::stable_partition(
            ::std::begin(entered_), ::std::end(entered_),
            [] (auto const *entity) { return Boolean_::False == entity->free; }
        );
        auto const end_ = ::std::end(entered_);
        auto result_ = Event_{.list = {}, .state = false};
        if (end_ != begin_) result_.list.splice(
            ::std::begin(result_.list), entered_, begin_, end_
        );
        return result_;
    } ();

    auto &&entered_ = [&queue_ = context.queue] {
        auto result_ = Event_{
            .list = ::std::move(queue_), .state = true
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

    if (! removed_.list.empty()) context.channel.dispatch(
        ::std::move(removed_)
    );

    if (! entered_.list.empty()) context.channel.dispatch(
        ::std::move(entered_)
    );
}

inline static auto on_shutdown(auto &context) noexcept(true) {
    context.state = false;
    context.queue.clear();
    if (context.entered.empty()) return;
    context.channel.dispatch(typename ::std::decay_t<
        decltype(context.channel)
    >::Event{
        .list = ::std::move(context.entered), .state = false
    });
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
        context_.reset(new ::std::decay_t<decltype(*context_)>{});

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::After,
            root_::core::bindings::engine::CreateEntity
        >([
            &globals_ = core_.api.meta.globals,
            &context_ = *context_, &log_ = core_.log
        ] () {
            if (! globals_) return;
            try { this_::private_::on_create(*globals_, context_); }
            catch (...) {
                log_.write<LogLevel_::Error>()
                << root_::exception::generate_details();
            }
        }));

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::Before,
            root_::core::bindings::engine::RemoveEntity
        >([&context_ = *context_] (auto *entity) {
            this_::private_::on_remove(context_, entity);
        }));

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::After,
            root_::core::bindings::engine::CreateNamedEntity
        >([
            &globals_ = core_.api.meta.globals,
            &context_ = *context_, &log_ = core_.log
        ] (auto) {
            if (! globals_) return;
            try { this_::private_::on_create(*globals_, context_); }
            catch (...) {
                log_.write<LogLevel_::Error>()
                << root_::exception::generate_details();
            }
        }));

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::After,
            root_::core::bindings::game::Init
        >([&context_ = *context_] () {
            this_::private_::on_init(context_);
        }));

        context_->subscriptions.push_front(bindings_->inject<
            root_::core::binding::Phase::After,
            root_::core::bindings::game::Spawn
        >([&context_ = *context_, &log_ = core_.log] (auto *entity) {
            try { this_::private_::on_spawn(context_, entity); }
            catch (...) {
                log_.write<LogLevel_::Error>()
                << root_::exception::generate_details();
            }
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

} // namespace p5::refrigerator::features::trusted_entities
