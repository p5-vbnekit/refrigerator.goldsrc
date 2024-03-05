#include <list>
#include <string>
#include <utility>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <type_traits>

#include <boost/algorithm/string.hpp>

#include <p5/lambda/utils.hxx>
#include <p5/lambda/engine.hxx>
#include <p5/lambda/metamod.hxx>

#include "log.hxx"
#include "exception.hxx"


namespace p5::refrigerator::log {
namespace private_ {

namespace parent_ = this_;
namespace this_ = parent_::private_;

using Message = parent_::Interface::Message;
namespace message = ::p5::lambda::utils::log::message;

inline constexpr static auto * metamod_sink(
    this_::Message::Level level,
    parent_::Context::Meta::Api const &api
) noexcept(true) {
    if (::std::decay_t<decltype(level)>::Info > level) {
        if (api.log_error) return api.log_error;
    }
    else if (::std::decay_t<decltype(level)>::Debug > level) {
        if (api.log_message) return api.log_message;
    }
    else if (api.log_developer) return api.log_developer;
    return api.log_console;
}

inline static ::std::string make_tag(
    char const *plugin, this_::Message::Level level
) noexcept(false) {
    auto &&buffer_ = ::std::list<::std::string>{};
    if (plugin && (0 != *plugin)) buffer_.emplace_back(plugin);
    auto const * const level_ = this_::message::level::to_string(level);
    if (level_ && (0 != level_)) buffer_.emplace_back(level_);
    return ::boost::algorithm::join(::std::move(buffer_), "|");
}

inline static auto compile_text(
    this_::Message &message
) noexcept(false) {
    auto text_ = [
        message_ = this_::message::text::normalizer::make(message.text)
    ] () mutable {
        ::std::ostringstream stream_;
        for (auto const &chunk_: message_) stream_ << chunk_.reference;
        return stream_.str();
    } ();
    ::boost::algorithm::trim(text_);
    if (! message.location) return text_;
    thread_local static auto const root_ = [] () -> ::std::string {
        auto const file_ = ::std::decay_t<
            decltype(*(message.location))
        >::current().file;
        if (file_.empty()) return {};
        auto const path_ = ::std::filesystem::path{file_};
        if (! path_.is_absolute()) return {};
        return path_.parent_path().string();
    } ();
    auto location_ = message.location->to_string(root_);
    if (location_.empty()) return text_;
    ::std::ostringstream stream_;
    stream_ << "<" << ::std::move(location_);
    if (text_.empty()) stream_ << ">";
    else stream_ << "> " << ::std::move(text_);
    return stream_.str();
}

} // namespace private_


this_::Type::Type(Context &&context) noexcept(true): context_{context} {}

void this_::Type::write_(Interface::Message &&message) const noexcept(true) {
    try {
        auto plugin_ = context_.meta.plugin;
        auto &&tag_ = this_::private_::make_tag(plugin_.log_tag, message.level);
        auto &&text_ = this_::private_::compile_text(message);
        if (auto * const sink_ = this_::private_::metamod_sink(
            message.level, context_.meta.api
        )) {
            plugin_.log_tag = tag_.c_str();
            sink_(&plugin_, "%s", text_.c_str());
            return;
        }
        if (auto * const sink_ = context_.engine.serverPrint) {
            ::std::ostringstream stream_;
            if (tag_.empty()) {
                if (text_.empty()) return;
                stream_ << ::std::move(text_) << ::std::endl;
                sink_(stream_.str().c_str());
                return;
            }
            stream_ << "[" << ::std::move(tag_);
            if (text_.empty()) stream_ << "]"  << ::std::endl;
            else stream_ << "]: " << ::std::move(text_)  << ::std::endl;
            sink_(stream_.str().c_str());
            return;
        }
        if (tag_.empty()) {
            if (text_.empty()) return;
            ::std::clog << ::std::move(text_) << ::std::endl << ::std::flush;
            return;
        }
        if (text_.empty()) {
            ::std::clog << "[" << ::std::move(tag_) << "]"
            << ::std::endl << ::std::flush;
            return;
        }
        [level_ = message.level] () -> auto & {
            if (
                ::std::decay_t<decltype(level_)>::Warning > level_
            ) return ::std::clog;
            return ::std::cerr;
        } () << "[" << ::std::move(tag_) << "]: " << ::std::move(text_)
        << ::std::endl << ::std::flush;
    }

    catch (...) { try {
        ::std::ostringstream stream_;
        for (
            auto const &chunk: parent_::exception::generate_details()
        ) stream_ << ::std::move(chunk.reference);
        if (stream_.view().empty()) return;
        stream_ << ::std::endl;
        ::std::cerr << stream_.view() << ::std::flush;
    } catch (...) {} }
}

} // namespace p5::refrigerator::log
