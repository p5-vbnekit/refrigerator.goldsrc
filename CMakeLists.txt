cmake_minimum_required(VERSION 3.5)

project("p5-refrigerator" LANGUAGES "CXX")

include("kconfig.cmake")
cmake_language(CALL "_kconfig::enable")

include("${PROJECT_SOURCE_DIR}/extra/lib/cmake/dependencies.cmake")

add_subdirectory("meta")
