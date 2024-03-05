cmake_minimum_required(VERSION 3.20)

include("CMakeParseArguments")

function("_auxiliary::get_target_namespace")
    unset("_arguments_OUTPUT")
    unset("_arguments_DIRECTORY")

    set("_arguments_OUTPUT" "${ARGV0}")
    if("" STREQUAL "${_arguments_OUTPUT}")
        message(FATAL_ERROR "First argument is empty (output variable name)")
    endif("" STREQUAL "${_arguments_OUTPUT}")

    unset("${_arguments_OUTPUT}")
    set("${_arguments_OUTPUT}" "")
    mark_as_advanced("${_arguments_OUTPUT}")

    cmake_parse_arguments(
        PARSE_ARGV 1
        "_arguments" "" "PROJECT;DIRECTORY" ""
    )

    if(_arguments_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown arguments: ${_arguments_UNPARSED_ARGUMENTS}")
    endif(_arguments_UNPARSED_ARGUMENTS)

    if(DEFINED "_arguments_DIRECTORY")
        cmake_path(
            ABSOLUTE_PATH "_arguments_DIRECTORY"
            BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            NORMALIZE OUTPUT_VARIABLE "_arguments_DIRECTORY"
        )
        cmake_path(IS_PREFIX "PROJECT_SOURCE_DIR" "${_arguments_DIRECTORY}" NORMALIZE "_is_prefix")
        if(NOT _is_prefix)
            message(FATAL_ERROR "DIRECTORY is not relative to PROJECT_SOURCE_DIR")
        endif(NOT _is_prefix)
        string(REGEX REPLACE "/+$" "" "_arguments_DIRECTORY" "${_arguments_DIRECTORY}")
    else()
        cmake_path(SET "_arguments_DIRECTORY" NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}")
    endif(DEFINED "_arguments_DIRECTORY")

    cmake_path(
        RELATIVE_PATH "_arguments_DIRECTORY"
        BASE_DIRECTORY "${PROJECT_SOURCE_DIR}"
        OUTPUT_VARIABLE "_arguments_DIRECTORY"
    )
    string(REGEX REPLACE "[^_A-z\-]" "." "${_arguments_OUTPUT}" "${_arguments_DIRECTORY}")

    if(NOT "" STREQUAL "${PROJECT_NAME}")
        string(PREPEND "${_arguments_OUTPUT}" "${PROJECT_NAME}+")
    endif(NOT "" STREQUAL "${PROJECT_NAME}")

    set("${_arguments_OUTPUT}" "${${_arguments_OUTPUT}}" PARENT_SCOPE)
endfunction("_auxiliary::get_target_namespace")
