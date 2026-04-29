# Generated Flex/Bison sources live in the build tree.
bison_target(DslParser
        parser.y
        ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp
        COMPILE_FLAGS "--language=c++ --locations")

flex_target(DslScanner
        scanner.l
        ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)

# Ensure Bison runs before Flex (scanner.l #includes parser.hpp).
add_flex_bison_dependency(DslScanner DslParser)

add_library(dsl_frontend STATIC
        ${BISON_DslParser_OUTPUTS}
        ${FLEX_DslScanner_OUTPUTS})

target_include_directories(dsl_frontend
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR})  # parser.hpp lives here

target_link_libraries(dsl_frontend PUBLIC dsl_music)

# Suppress noisy warnings in generated code (register, sign-compare, etc.).
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(dsl_frontend PRIVATE
            -Wno-deprecated-register
            -Wno-sign-compare
            -Wno-unused-function
            -Wno-unused-but-set-variable)
endif ()
