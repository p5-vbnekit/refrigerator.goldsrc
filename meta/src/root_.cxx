#include <cstdint>

#include <sstream>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <type_traits>

#include "core.hxx"
#include "root_.hxx"
#include "exception.hxx"


namespace p5::refrigerator {

// `GiveFnptrsToDll` - receive engine function table from engine.
// This appears to be the _first_ DLL routine called by the engine, so we do some setup operations here.
void inject(
    ::p5::lambda::engine::Functions const *functions,
    ::p5::lambda::engine::Globals *globals
) noexcept(true) {
    auto &core_ = this_::Core::instance();

    using LogLevel_ = ::std::decay_t<decltype(core_.log)>::Message::Level;

    core_.log.write<LogLevel_::Developer>()
    << "globals = " << static_cast<void const *>(globals)
    << ", functions = " << functions;

    try {
        if (! static_cast<bool>(globals)) throw ::std::invalid_argument{
            "globals pointer is null"
        };

        if (! static_cast<bool>(functions)) throw ::std::invalid_argument{
            "functions pointer is null"
        };

        core_.api.engine.globals = globals;
        core_.api.engine.functions = *functions;
    }

    catch(...) {
        core_.log.write<LogLevel_::Error>(
            this_::exception::generate_details()
        );

        return;
    }

    core_.log.write<LogLevel_::Developer>() << "success";
}

// `Meta_Query` - metamod requesting info about this plugin:
// - `version`      (given)     interface_version metamod is using
// - `plugin`       (requested) struct with info about plugin
// - `functions`    (given)     table of utility functions provided by metamod
::p5::lambda::common::IntegerBoolean query(
    char const *version,
    ::p5::lambda::metamod::plugin::Info **plugin,
    ::p5::lambda::metamod::Functions const *functions
) noexcept(true) {
    auto &core_ = this_::Core::instance();

    using LogLevel_ = ::std::decay_t<decltype(core_.log)>::Message::Level;

    core_.log.write<LogLevel_::Developer>()
    << "interface version = \"" << (version ? version : "nullptr")
    << "\", plugin = " << static_cast<void const *>(plugin)
    << ", functions = " << static_cast<void const *>(functions);

    try {
        auto const version_ = [pointer_ = version] {
            if (! pointer_) throw ::std::logic_error{
                "metamod api version pointer is null"
            };
            return ::std::string_view{pointer_};
        } ();

        if (version_.empty()) throw ::std::invalid_argument{
            "metamod api version is empty"
        };

        if (! static_cast<bool>(plugin)) throw ::std::invalid_argument{
            "plugin pointer is null"
        };

        if (! static_cast<bool>(functions)) throw ::std::invalid_argument{
            "functions pointer is null"
        };

        auto &api_ = core_.api;

        auto const plugin_version_ = [
            pointer_ = api_.meta.plugin.info.interface_version
        ] {
            if (! pointer_) throw ::std::logic_error{
                "plugin api version pointer is null"
            };
            return ::std::string_view{pointer_};
        } ();

        if (plugin_version_.empty()) throw ::std::logic_error{
            "plugin api version is empty"
        };

        *plugin = &api_.meta.plugin.info;

        if (version_ != plugin_version_) {
            ::std::ostringstream stream_;
            stream_ << "interface version mismatch: plugin["
            << plugin_version_ << "] != metamod[" << version_ << "]";
            throw ::std::invalid_argument{stream_.view().data()};
        }

        if (! api_.engine.globals) throw ::std::logic_error{
            "invalid state: `GiveFnptrsToDll` success expected"
        };

        api_.meta.functions = *functions;
    }

    catch(...) {
        core_.log.write<LogLevel_::Error>(
            this_::exception::generate_details()
        );

        return ::p5::lambda::common::IntegerBoolean::False;
    }

    core_.log.write<LogLevel_::Developer>() << "success";

    return ::p5::lambda::common::IntegerBoolean::True;
}

// `Meta_Attach` - metamod attaching plugin to the server:
// - `phase`    (given)     current phase, ie during map, during changelevel, or at startup
// - `plugin`   (requested) table of function tables this plugin catches
// - `globals`  (given)     global vars from metamod
// - `game`     (given)     copy of function tables from game shared module
::p5::lambda::common::IntegerBoolean attach(
    ::p5::lambda::metamod::plugin::LoadTime phase,
    ::p5::lambda::metamod::plugin::Functions *plugin,
    ::p5::lambda::metamod::Globals *globals,
    ::p5::lambda::game::functions::Pointers const *game
) noexcept(true) {
    auto &core_ = this_::Core::instance();

    using LogLevel_ = ::std::decay_t<decltype(core_.log)>::Message::Level;

    core_.log.write<LogLevel_::Developer>()
    << "phase = " << static_cast<::std::size_t>(phase)
    << ", plugin = " << static_cast<void const *>(plugin)
    << ", globals = " << static_cast<void const *>(globals)
    << ", game = " << static_cast<void const *>(game);

    try {
        if (! static_cast<bool>(plugin)) throw ::std::invalid_argument{
            "plugin pointer is null"
        };

        if (! static_cast<bool>(globals)) throw ::std::invalid_argument{
            "globals pointer is null"
        };

        if (! static_cast<bool>(game)) throw ::std::invalid_argument{
            "game pointer is null"
        };

        auto const * const standard_ = game->standard;
        if (! static_cast<bool>(standard_)) throw ::std::invalid_argument{
            "standard game functions pointer is null"
        };

        auto const * const extension_ = game->extension;
        if (! static_cast<bool>(extension_)) throw ::std::invalid_argument{
            "\new\" game functions pointer is null"
        };

        if (! core_.api.engine.globals) throw ::std::logic_error{
            "invalid state: `GiveFnptrsToDll` success expected"
        };

        *plugin = core_.api.meta.plugin.functions;

        core_.api.meta.globals = globals;
        core_.api.game.functions.standard = *standard_;
        core_.api.game.functions.extension = *extension_;

        try { core_.load(); } catch (...) { ::std::throw_with_nested(
            ::std::runtime_error{"unable to load core"}
        ); }
    }

    catch(...) {
        core_.log.write<LogLevel_::Error>(
            this_::exception::generate_details()
        );

        return ::p5::lambda::common::IntegerBoolean::False;
    }

    core_.log.write<LogLevel_::Developer>() << "success";

    return ::p5::lambda::common::IntegerBoolean::True;
}

// `Meta_Detach` - metamod detaching plugin from the server:
// - `phase`    (given) current phase, ie during map, etc
// - `reason`   (given) why detaching (refresh, console unload, forced unload, etc)
::p5::lambda::common::IntegerBoolean detach(
    ::p5::lambda::metamod::plugin::LoadTime phase,
    ::p5::lambda::metamod::plugin::UnloadReason reason
) noexcept(true) {
    auto &core_ = this_::Core::instance();

    using LogLevel_ = ::std::decay_t<decltype(core_.log)>::Message::Level;

    core_.log.write<LogLevel_::Developer>()
    << "phase = " << static_cast<::std::size_t>(phase)
    << ", reason = " << static_cast<::std::size_t>(reason);

    try {
        try { core_.unload(); } catch (...) {
            core_.api.meta.globals = nullptr;
            core_.api.game.functions = {};
            ::std::throw_with_nested(
                ::std::runtime_error{"unable to unload core"}
            );
        }

        core_.api.meta.globals = nullptr;
        core_.api.game.functions = {};
    }

    catch(...) {
        core_.log.write<LogLevel_::Error>(
            this_::exception::generate_details()
        );

        return ::p5::lambda::common::IntegerBoolean::False;
    }

    core_.log.write<LogLevel_::Developer>() << "success";

    return ::p5::lambda::common::IntegerBoolean::True;
}

} // namespace p5::refrigerator
