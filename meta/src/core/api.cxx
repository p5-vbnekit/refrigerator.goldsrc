#include <string_view>

#include "api.hxx"


namespace p5::refrigerator::core::api {

this_::Type factory() noexcept(true) {
    constexpr auto instance_ = this_::Type{.meta = {.plugin = {.info = {
#ifdef PLUGIN_NAME_
        .name = PLUGIN_NAME_,
#endif
#ifdef PLUGIN_VERSION_
        .version = PLUGIN_VERSION_,
#endif
#ifdef PLUGIN_TIMESTAMP_
        .date = PLUGIN_TIMESTAMP_,
#endif
        .author = "p5-vbnekit <vbnekit@gmail.com>",
        .url = "https://github.com/p5-vbnekit/refrigerator.goldsrc",
        .log_tag = "P5-REFRIGERATOR"
    }}}};

    if constexpr (true) {
        constexpr auto const validate_ = [] (auto const *pointer) consteval {
            return pointer && (! ::std::string_view{pointer}.empty());
        };
        static_assert(validate_(instance_.meta.plugin.info.interface_version));
        static_assert(validate_(instance_.meta.plugin.info.name));
        static_assert(validate_(instance_.meta.plugin.info.version));
        static_assert(validate_(instance_.meta.plugin.info.date));
        static_assert(validate_(instance_.meta.plugin.info.author));
        static_assert(validate_(instance_.meta.plugin.info.url));
        static_assert(validate_(instance_.meta.plugin.info.log_tag));
    }

    return instance_;
}

} // namespace p5::refrigerator::core::api
