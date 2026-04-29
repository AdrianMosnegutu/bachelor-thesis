cmake_minimum_required(VERSION 3.20)
project(dsl CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif ()

# Export compile_commands.json for IDE integration (clangd, CLion, VSCode).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(BISON 3.8 REQUIRED)
find_package(FLEX 2.6 REQUIRED)

add_subdirectory(src)
add_subdirectory(apps)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
