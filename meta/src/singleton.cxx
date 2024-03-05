#include <memory>
#include <utility>
#include <sstream>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <type_traits>

#include <boost/core/demangle.hpp>
#include <boost/algorithm/string.hpp>

#include "exception.hxx"
#include "singleton.hxx"


namespace p5::refrigerator::singleton {

this_::Container & this_::Type::container() noexcept(false) {
    auto const index_ = container_.index();
    if (1 == index_) return ::std::get<1>(container_);
    if (0 == index_) if (
        auto const exception_ = ::std::get<0>(container_)
    ) ::std::rethrow_exception(exception_);
    throw ::std::logic_error{"invalid state"};
}

this_::Container const & this_::Type::container() const noexcept(false) {
    return const_cast<::std::decay_t<decltype(*this)> *>(this)->container();
}


this_::Type & this_::Type::instance() noexcept(true) {
    static Type instance_; return instance_;
}

this_::Type::Type() noexcept(true):
    api{parent_::api::factory()}, log{log_}, bindings{}, log_{{
        .meta = {.api = api.meta.functions, .plugin = api.meta.plugin.info},
        .engine = api.engine.functions
    }}
{
    bindings.apply(api.meta.plugin.functions);

    using LogLevel_ = ::std::decay_t<decltype(log)>::Message::Level;

    try { container_.emplace<1>(); }

    catch (...) {
        auto const exception_ = ::std::current_exception();
        container_.emplace<0>(exception_);
        log.write<LogLevel_::Error>() << "container error: "
        << parent_::exception::generate_details(exception_);
        return;
    }

    auto &container_ = ::std::get<1>(this->container_);

    try { subscription_ = ::std::move(container_.on_error([this] (
        auto const &key, auto const &exception
    ) {
        log.write<LogLevel_::Error>() << "container error [" <<
        ::boost::core::demangle(key.name()) << "]: "
        << parent_::exception::generate_details(exception);
    }).take()); } catch (...) { log.write<LogLevel_::Error>(
        ::p5::lambda::utils::exception::details()
    ); }

    try { container_.assign(api); } catch (...) { log.write(
        LogLevel_::Error,
        parent_::exception::generate_details()
    ); }

    try { container_.assign(bindings); } catch (...) { log.write(
        LogLevel_::Error,
        parent_::exception::generate_details()
    ); }
}

} // namespace p5::refrigerator
