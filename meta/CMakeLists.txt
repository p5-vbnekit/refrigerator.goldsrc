cmake_minimum_required(VERSION 3.5)

include("${PROJECT_SOURCE_DIR}/extra/lib/cmake/auxiliary.cmake")

cmake_language(CALL "_auxiliary::get_target_namespace" "_target")
mark_as_advanced("_target")

add_library("${_target}" MODULE)

file(
    GLOB_RECURSE "_sources"
    LIST_DIRECTORIES FALSE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)
mark_as_advanced("_sources")
target_sources("${_target}" PRIVATE ${_sources})

target_compile_features("${_target}" PRIVATE "cxx_std_20")

set_target_properties("${_target}" PROPERTIES
    PREFIX ""
    OUTPUT_NAME "${PROJECT_NAME}"
    CXX_STANDARD "20"
    CXX_STANDARD_REQUIRED TRUE
    DEFINE_SYMBOL "P5_REFRIGERATOR_IMPLEMENTATION"
    C_VISIBILITY_PRESET "hidden"
    CXX_VISIBILITY_PRESET "hidden"
    VISIBILITY_INLINES_HIDDEN TRUE
    POSITION_INDEPENDENT_CODE TRUE
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/.output/addons/metamod/plugins"
)

if(MSVC)
    if(SIZE_OF_VOID_P EQUAL 4)
        target_compile_options("${_target}" PRIVATE "/arch:sse" "/arch:sse2")
    endif(SIZE_OF_VOID_P EQUAL 4)
else(MSVC)
    target_compile_options("${_target}" PRIVATE "-mtune=generic" "-msse" "-msse2")
    target_link_options("${_target}" PRIVATE "-static-libgcc")
    target_link_options("${_target}" PRIVATE "-static-libstdc++")
    target_link_options("${_target}" PRIVATE "-Wl,--no-undefined")
endif(MSVC)

cmake_language(CALL "_auxiliary::get_version" "_version")
cmake_language(CALL "_auxiliary::get_timestamp" "_timestamp")
mark_as_advanced("_version")
mark_as_advanced("_timestamp")

unset("_info")
list(
    APPEND "_info"
    "PLUGIN_NAME_=\"${PROJECT_NAME}\""
    "PLUGIN_VERSION_=\"${_version}\""
    "PLUGIN_TIMESTAMP_=\"${_timestamp}\""
)
set_source_files_properties(
    "src/core/api.cxx" PROPERTIES COMPILE_DEFINITIONS "${_info}"
)

set_source_files_properties(
    "src/core/log.cxx" PROPERTIES COMPILE_DEFINITIONS
    "$<$<CONFIG:Debug>:DEBUG_MODE=true>"
)

target_link_libraries(
    "${_target}" PRIVATE
    "p5-lambda"
    "Boost::boost"
)
