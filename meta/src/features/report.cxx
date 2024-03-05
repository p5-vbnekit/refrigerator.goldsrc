#include <cmath>

#include <sstream>
#include <optional>
#include <string_view>
#include <type_traits>

#include "../binding.hxx"
#include "../bindings.hxx"
#include "../exception.hxx"
#include "../singleton.hxx"

#include "report.hxx"


namespace p5::refrigerator::features::report {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

namespace root_ = parent_::parent_::parent_;

template <class Vector> inline static
auto accumulate_vectors_(Vector const &a, Vector const &b) noexcept(true) {
    return ::std::decay_t<Vector>{
        .x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z
    };
}

template <class Vector> inline static
auto reverse_vector_(Vector const &vector) noexcept(true) {
    return ::std::decay_t<Vector>{
        .x = - vector.x, .y = - vector.y, .z = - vector.z
    };
}

template <class Vector> inline static
auto vector_length_(Vector const &vector) noexcept(true) {
    return ::std::sqrt((
        vector.x * vector.x
    ) + (
        vector.y * vector.y
    ) + (
        vector.z * vector.z
    ));
}

template <class Vector> inline static
auto format_vector_(Vector const &vector) noexcept(false) {
    ::std::ostringstream stream_;
    stream_ << vector.x << " " << vector.y << " " << vector.z;
    return stream_.str();
}

template <class Client> inline static
auto trace_target_(Client const &client) noexcept(true) {
    using Trace_ = ::p5::lambda::common::trace::Result;
    using Entity_ = ::std::decay_t<decltype(*(Trace_::pHit))>;
    using Vector_ = ::std::decay_t<decltype(Trace_::vecEndPos)>;
    struct Payload_ final { Entity_ &entity; Vector_ position; };
    auto result_ = ::std::optional<Payload_>{};

    auto const &engine_ = root_::Singleton::instance().api.engine;
    if (! engine_.globals) return result_;
    if (! engine_.functions.makeVectors) return result_;

    auto const begin_ = this_::accumulate_vectors_(
        client.v.origin, client.v.view_ofs
    );
    auto end_ = [&engine_, &client, &begin_] {
        engine_.functions.makeVectors(&(client.v.v_angle.x));
        auto vector_ = engine_.globals->v_forward;
        vector_.x *= 8192;
        vector_.y *= 8192;
        vector_.z *= 8192;
        return this_::accumulate_vectors_(begin_, vector_);
    } ();

    if (engine_.functions.traceLine) [&] {
        auto tr_ = Trace_{};
        engine_.functions.traceLine(&(begin_.x), &(end_.x), 0, &client, &tr_);
        if (tr_.pHit && (1 > tr_.flFraction)) result_.emplace(Payload_{
            .entity = *(tr_.pHit), .position = (end_ = tr_.vecEndPos)
        });
    } ();

    if ([&engine_ = engine_.functions] {
        if (! engine_.numberOfEntities) return false;
        if (! engine_.pEntityOfEntityIndex) return false;
        return static_cast<bool>(engine_.traceModel);
    } ()) [
        &engine_ = engine_.functions,
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
            auto * const entity_ = engine_.pEntityOfEntityIndex(index_);
            if (! entity_) continue;
            if (0 == entity_->v.model) continue;
            if (! (0 < entity_->v.modelindex)) continue;
            if (client_ == entity_) continue;
            if (result_ && (&(result_->entity) == entity_)) continue;
            auto trace_ = Trace_{};
            engine_.traceModel(&(begin_.x), &(end_.x), 0, entity_, &trace_);
            if (entity_ != trace_.pHit) continue;
            if (! (1 > trace_.flFraction)) continue;
            auto const &position_ = trace_.vecEndPos;
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

template <class Client> inline static
auto handle_client_command_(Client &client) noexcept(false) {
    auto const &singleton_ = root_::Singleton::instance();
    auto const &engine_ = singleton_.api.engine;
    if (! engine_.functions.cmd_Argc) return;
    if (! engine_.functions.cmd_Argv) return;
    if (2 < engine_.functions.cmd_Argc()) return;
    if constexpr (true) {
        auto const * pointer_ = engine_.functions.cmd_Argv(0);
        if (! pointer_) return;
        if ("refrigerator" != ::std::string_view{pointer_}) return;
        pointer_ = engine_.functions.cmd_Argv(1);
        if ("report" != ::std::string_view{pointer_}) return;
    }
    using PrintType_ = ::p5::lambda::common::PrintType;
    engine_.functions.clientPrintf(&client, PrintType_::Console, ([
        &client, &engine_
    ] {
        auto stream_ = ::std::ostringstream{};
        if (auto const time_ = [&engine_] {
            auto value_ = ::std::optional<
                ::std::decay_t<decltype(engine_.functions.time())>
            >{};
            if (engine_.functions.time) value_.emplace(engine_.functions.time());
            else if (engine_.globals) value_.emplace(engine_.globals->time);
            return value_;
        } ()) stream_ << "time: " << *time_ << "\n";
        stream_ << "position: "<< this_::format_vector_(
            client.v.origin
        ) << "\n";
        if (auto const target_ = this_::trace_target_(client)) {
            stream_ << "target: ";
            auto const class_ = [
                &engine_ = engine_.functions, &target_ = target_->entity.v
            ] () -> ::std::string_view {
                if (! engine_.szFromIndex) return {};
                auto const * const pointer_ = engine_.szFromIndex(
                    target_.classname
                );
                if (! pointer_) return {};
                return pointer_;
            } ();
            if (class_.empty()) stream_ << "<unknown> at ";
            else stream_ << class_ << " @ ";
            stream_ << this_::format_vector_(target_->position) << "\n";
        }
        return stream_.str();
    } ()).c_str());
}

[[maybe_unused]] inline static void const * const injection_ = [] {
    auto &singleton_ = root_::Singleton::instance();

    auto const &log_ = singleton_.log;
    using LogLevel_ = ::std::decay_t<decltype(log_)>::Message::Level;

    try { singleton_.container().inject([&singleton_] {
        singleton_.bindings.inject<
            root_::binding::Phase::After,
            root_::bindings::game::ClientCommand
        >([] (auto *entity) {
            if (entity) this_::handle_client_command_(*entity);
        });
        return parent_::Type{};
    }); } catch(...) {
        log_.write<LogLevel_::Error>()
        << root_::exception::generate_details();
    }

    return nullptr;
} ();

} // namespace private_
} // namespace p5::refrigerator::features::report
