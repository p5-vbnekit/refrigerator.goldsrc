cmake_minimum_required(VERSION 3.23)

function("_auxiliary::get_version")
    unset("_output")

    set("_output" "${ARGV0}")
    if("" STREQUAL "${_output}")
        message(FATAL_ERROR "First argument is empty (output variable name)")
    endif("" STREQUAL "${_output}")

    unset("_temporary")

    if(DEFINED "PROJECT_VERSION")
        set("_temporary" "${PROJECT_VERSION}")
    endif(DEFINED "PROJECT_VERSION")

    if("" STREQUAL "${_temporary}")
        find_package("Git")
        if(Git_FOUND)
            execute_process(
                COMMAND "${GIT_EXECUTABLE}" "rev-parse" "--short" "HEAD"
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                OUTPUT_VARIABLE "_temporary"
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            if(NOT "" STREQUAL "${_temporary}")
                set("_temporary" "0.0+${_temporary}@git")
            endif(NOT "" STREQUAL "${_temporary}")
        endif(Git_FOUND)
    endif("" STREQUAL "${_temporary}")

    if("" STREQUAL "${_temporary}")
        string(TIMESTAMP "_temporary" "%s.%f")
        set("_temporary" "0.0+${_temporary}")
    endif("" STREQUAL "${_temporary}")

    if("" STREQUAL "${_temporary}")
        set("_temporary" "0.0+unknown")
    endif("" STREQUAL "${_temporary}")

    unset("${_output}")
    set("${_output}" "${_temporary}" PARENT_SCOPE)
endfunction("_auxiliary::get_version")
