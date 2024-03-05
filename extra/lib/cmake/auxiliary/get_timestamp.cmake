cmake_minimum_required(VERSION 3.23)

function("_auxiliary::get_timestamp")
    unset("_output")

    set("_output" "${ARGV0}")
    if("" STREQUAL "${_output}")
        message(FATAL_ERROR "First argument is empty (output variable name)")
    endif("" STREQUAL "${_output}")

    unset("_temporary")

    find_package("Git")
    if(Git_FOUND)
        execute_process(
            COMMAND "TZ=GMT" "${GIT_EXECUTABLE}" "show" "--no-patch" "--format=%ci" "HEAD"
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
            OUTPUT_VARIABLE "_temporary"
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif(Git_FOUND)

    if("" STREQUAL "${_temporary}")
        string(TIMESTAMP "_temporary" "%Y-%m-%dT%H:%M:%S")
    endif("" STREQUAL "${_temporary}")

    unset("${_output}")
    set("${_output}" "${_temporary}" PARENT_SCOPE)
endfunction("_auxiliary::get_timestamp")
