cmake_minimum_required(VERSION 3.5)

function("_auxiliary::kconfig")
    unset("_json")
    unset("_output")
    unset("_script")
    unset("_python3")
    unset("_base_directory")

    set("_output" "${ARGV0}")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/KConfig")
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/KConfig"
            "${CMAKE_CURRENT_BINARY_DIR}/.kconfig/KConfig"
            COPYONLY
        )
    endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/KConfig")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.kconfig")
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/.kconfig"
            "${CMAKE_CURRENT_BINARY_DIR}/.kconfig/.config"
            COPYONLY
        )
    endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.kconfig")

    get_filename_component(
        "_base_directory" "../../.." ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_FUNCTION_LIST_DIR}"
    )
    if("" STREQUAL "${_base_directory}")
        message(FATAL_ERROR "unable to make path to base directory")
    endif("" STREQUAL "${_base_directory}")

    set("_script" "${_base_directory}/utils/kconfig.py")

    if(NOT TARGET "Python3::Interpreter")
        find_package("Python3" COMPONENTS "Interpreter")
    endif(NOT TARGET "Python3::Interpreter")

    if(TARGET "Python3::Interpreter")
        get_target_property("_python3" "Python3::Interpreter" "LOCATION")
        if("" STREQUAL "${_python3}")
            message(FATAL_ERROR "unable to make path to python3")
        endif()
        if (EXISTS "${CMAKE_CURRENT_BINARY_DIR}/.kconfig/KConfig")
            execute_process(
                COMMAND "${_python3}" "--" "${_script}" "setconfig"
                WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.kconfig"
            )
        endif(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/.kconfig/KConfig")
    else()
        set("_python3" "python3")
        message(WARNING "Python3 not found")
    endif(TARGET "Python3::Interpreter")

    configure_file(
        "${_base_directory}/lib/kconfig.Makefile.in"
        "${CMAKE_CURRENT_BINARY_DIR}/.kconfig/Makefile"
        @ONLY
    )

    if("" STREQUAL "${_output}")
        return()
    endif("" STREQUAL "${_output}")

    set("_json" "")
    if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/.kconfig/.config.json")
        file(STRINGS "${CMAKE_CURRENT_BINARY_DIR}/.kconfig/.config.json" "_json")
    endif(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/.kconfig/.config.json")

    set("${_output}" "${_json}" PARENT_SCOPE)
endfunction("_auxiliary::kconfig")
