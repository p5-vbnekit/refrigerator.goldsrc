#include <cmath>

#include <sstream>
#include <optional>
#include <string_view>
#include <type_traits>

#include "../core.hxx"
#include "namespace_.hxx"
#include "../exception.hxx"


namespace p5::refrigerator::features::report {

namespace parent_ = this_;
namespace this_ = parent_::report;

namespace root_ = parent_::parent_;

template <class Vector> inline static
auto accumulate_vectors_(Vector const &a, Vector const &b) noexcept(true) {
    return ::std::decay_t<Vector>{
        .x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z
    };
}

inline static auto reverse_vector_(auto const &vector) noexcept(true) {
    return ::std::decay_t<decltype(vector)>{
        .x = - vector.x, .y = - vector.y, .z = - vector.z
    };
}

inline static auto vector_length_(auto const &vector) noexcept(true) {
    return ::std::sqrt((
        vector.x * vector.x
    ) + (
        vector.y * vector.y
    ) + (
        vector.z * vector.z
    ));
}

inline static auto format_vector_(auto const &vector) noexcept(false) {
    ::std::ostringstream stream_;
    stream_ << vector.x << " " << vector.y << " " << vector.z;
    return stream_.str();
}

inline static auto trace_target_(
    auto const &engine, auto const &client
) noexcept(true) {
    using Trace_ = ::p5::lambda::common::trace::Result;
    using Entity_ = ::std::decay_t<decltype(*(Trace_::entity))>;
    using Vector_ = ::std::decay_t<decltype(Trace_::final_position)>;
    struct Payload_ final { Entity_ &entity; Vector_ position; };
    auto result_ = ::std::optional<Payload_>{};

    if (! engine.globals) return result_;
    if (! engine.functions.makeVectors) return result_;

    auto const begin_ = this_::accumulate_vectors_(
        client.variables.origin, client.variables.view_offset
    );
    auto end_ = [&engine, &client, &begin_] {
        engine.functions.makeVectors(&(client.variables.view_angle.x));
        auto vector_ = engine.globals->forward;
        vector_.x *= 8192;
        vector_.y *= 8192;
        vector_.z *= 8192;
        return this_::accumulate_vectors_(begin_, vector_);
    } ();

    if (engine.functions.traceLine) [&] {
        auto tr_ = Trace_{};
        engine.functions.traceLine(&(begin_.x), &(end_.x), 0, &client, &tr_);
        if (tr_.entity && (1 > tr_.fraction)) result_.emplace(Payload_{
            .entity = *(tr_.entity), .position = (end_ = tr_.final_position)
        });
    } ();

    if ([&engine = engine.functions] {
        if (! engine.numberOfEntities) return false;
        if (! engine.entityOfEntityIndex) return false;
        return static_cast<bool>(engine.traceModel);
    } ()) [
        &engine_ = engine.functions,
        client_ = &client,
        &result_, &begin_, &end_
    ] {
        auto const entities_ = engine_.numberOfEntities();
        if (! (0 < entities_)) return;
        auto const reversed_ = this_::reverse_vector_(begin_);
        auto limit_ = this_::vector_length_(
            this_::accumulate_vectors_(end_, reversed_)
        );
        auto *better_ = static_cast<Entity_ *>(nullptr);
        for (int index_ = 1; index_ < entities_; index_++) {
            auto * const entity_ = engine_.entityOfEntityIndex(index_);
            if (! entity_) continue;
            if (0 == entity_->variables.model) continue;
            if (! (0 < entity_->variables.model_index)) continue;
            if (client_ == entity_) continue;
            if (result_ && (&(result_->entity) == entity_)) continue;
            auto trace_ = Trace_{};
            engine_.traceModel(&(begin_.x), &(end_.x), 0, entity_, &trace_);
            if (entity_ != trace_.entity) continue;
            if (! (1 > trace_.fraction)) continue;
            auto const &position_ = trace_.final_position;
            auto const distance_ = this_::vector_length_(
                this_::accumulate_vectors_(end_ = position_, reversed_)
            );
            if (! (distance_ < limit_)) continue;
            better_ = entity_;
            limit_ = distance_;
            end_ = position_;
        }
        if (better_) result_.emplace(Payload_{
            .entity = *better_, .position = end_
        });
    } ();

    return result_;
}

inline static auto on_client_command_(
    auto const &engine, auto &client
) noexcept(false) {
    if (! engine.functions.cmd_Argc) return;
    if (! engine.functions.cmd_Argv) return;
    if (2 != engine.functions.cmd_Argc()) return;
    if constexpr (true) {
        auto const * pointer_ = engine.functions.cmd_Argv(0);
        if (! pointer_) return;
        if ("refrigerator" != ::std::string_view{pointer_}) return;
        pointer_ = engine.functions.cmd_Argv(1);
        if ("report" != ::std::string_view{pointer_}) return;
    }
    using PrintType_ = ::p5::lambda::common::PrintType;
    engine.functions.clientPrintf(&client, PrintType_::Console, ([
        &client, &engine
    ] {
        auto stream_ = ::std::ostringstream{};
        if (auto const time_ = [&engine] {
            auto value_ = ::std::optional<
                ::std::decay_t<decltype(engine.functions.time())>
            >{};
            if (engine.functions.time) value_.emplace(engine.functions.time());
            else if (engine.globals) value_.emplace(engine.globals->time);
            return value_;
        } ()) stream_ << "time: " << *time_ << "\n";
        stream_ << "position: " << this_::format_vector_(
            client.variables.origin
        ) << "\n";
        if (auto const target_ = this_::trace_target_(engine, client)) {
            auto const difference_ = this_::accumulate_vectors_(
                target_->position, this_::reverse_vector_(
                    client.variables.origin
                )
            );
            stream_ << "distance: "<< this_::vector_length_(
                difference_
            ) << "\n";
            stream_ << "target: ";
            auto const class_ = [
                &engine_ = engine.functions,
                &target_ = target_->entity.variables
            ] () -> ::std::string_view {
                if (! engine_.szFromIndex) return {};
                auto const * const pointer_ = engine_.szFromIndex(
                    target_.class_name
                );
                if (! pointer_) return {};
                return pointer_;
            } ();
            if (class_.empty()) stream_ << "<unknown> #";
            else stream_ << class_ << " #";
            stream_ << [
                &getter_ = engine.functions.indexOfEntityDictionary,
                &entity_ = target_->entity
            ] {
                using Value_ = ::std::decay_t<decltype(getter_(&entity_))>;
                if (! getter_) return static_cast<Value_>(0);
                return getter_(&entity_);
            } () << " ";
            stream_ << static_cast<void const *>(&(target_->entity)) << " @ ";
            stream_ << this_::format_vector_(target_->position) << "\n";
        }
        return stream_.str();
    } ()).c_str());
}

[[maybe_unused]] inline static void const * const injection_ = [] {
    auto &core_ = this_::root_::Core::instance();

    try { core_.inject({.load = [&core_] { core_.bindings()->inject<
        this_::root_::core::binding::Phase::After,
        this_::root_::core::bindings::game::ClientCommand
    >([&engine_ = core_.api.engine] (auto *entity) {
        if (entity) this_::on_client_command_(engine_, *entity);
    }).pin(); }}); } catch (...) {
        core_.invalidate();
        auto const &log_ = core_.log;
        using LogLevel_ = ::std::decay_t<decltype(log_)>::Message::Level;
        log_.write<LogLevel_::Error>()
        << this_::root_::exception::generate_details();
    }

    return nullptr;
} ();

} // namespace p5::refrigerator::features::report
