#pragma once

#include <string>
#include <utility>
#include <optional>
#include <exception>
#include <type_traits>

#include <boost/algorithm/string.hpp>

#include <p5/lambda/utils/log.hxx>
#include <p5/lambda/utils/generator.hxx>
#include <p5/lambda/utils/exception.hxx>

#include "namespace_.hxx"


namespace p5::refrigerator {
namespace exception {

namespace parent_ = this_;
namespace this_ = parent_::exception;

template <class T> inline static
auto generate_details(T &&exception) noexcept(true) {
    namespace utils_ = ::p5::lambda::utils;
    namespace chunk_ = utils_::log::message::text::chunk;

    using Chunk_ = ::std::decay_t<decltype(chunk_::make())>;

    struct State_ final {
        ::std::optional<Chunk_> value = ::std::nullopt;
        ::std::string buffer = {};
        ::std::exception_ptr pointer;
    };

    return utils_::generator::function::make([
        state_ = State_{.pointer = [&exception] {
        if constexpr (::std::is_base_of_v<
            ::std::exception, ::std::decay_t<T>
        >) return ::std::make_exception_ptr(::std::forward<T>(exception));
        else return ::std::forward<T>(exception);
        } ()}
    ] () mutable -> Chunk_ const * {
        if (state_.pointer) try {
            if (state_.value) {
                thread_local static auto const new_line_ = chunk_::make("\n");
                state_.value.reset();
                return &new_line_;
            }
            namespace exception_ = utils_::exception;
            state_.buffer = exception_::details(state_.pointer);
            state_.pointer = exception_::next(state_.pointer);
            ::boost::algorithm::trim(state_.buffer);
            if (state_.buffer.empty()) {
                thread_local static auto const unknown_ = chunk_::make(
                    "Unknown exception."
                );
                state_.value.emplace(unknown_);
            } else state_.value.emplace(chunk_::make(state_.buffer));
            return &(*(state_.value));
        } catch (...) { state_.pointer = {}; }
        return nullptr;
    });
}

inline static auto generate_details() noexcept(true) {
    return this_::generate_details(::std::current_exception());
}

} // namepsace exception
} // namespace p5::refrigerator
