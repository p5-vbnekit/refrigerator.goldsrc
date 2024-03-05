cmake_minimum_required(VERSION 3.5)

function("_auxiliary::include_all")
    unset("_directory")
    set("_directory" "${ARGV0}")

    if("" STREQUAL "${_directory}")
        message(FATAL_ERROR "First argument is empty (directory path)")
    endif("" STREQUAL "${_directory}")

    cmake_path(NORMAL_PATH "_directory" OUTPUT_VARIABLE "_directory")
    if("" STREQUAL "${_directory}")
        message(FATAL_ERROR "_directory is empty")
    endif("" STREQUAL "${_directory}")

    unset("_modules")
    file(GLOB "_modules" LIST_DIRECTORIES FALSE "${_directory}/*.cmake")

    foreach("_module" IN LISTS "_modules")
        include("${_module}")
    endforeach("_module" IN LISTS "_modules")
endfunction("_auxiliary::include_all")
