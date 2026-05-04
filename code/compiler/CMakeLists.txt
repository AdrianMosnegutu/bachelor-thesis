cmake_minimum_required(VERSION 3.20)
project(dsl CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(DSL_ENABLE_COVERAGE "Build compiler targets with coverage instrumentation" OFF)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif ()

# Export compile_commands.json for IDE integration (clangd, CLion, VSCode).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DslTargets)

find_package(BISON 3.8 REQUIRED)
find_package(FLEX 2.6 REQUIRED)

if(DSL_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    else()
        message(FATAL_ERROR "DSL_ENABLE_COVERAGE requires GCC or Clang")
    endif()
endif()

add_subdirectory(src)
add_subdirectory(apps)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
