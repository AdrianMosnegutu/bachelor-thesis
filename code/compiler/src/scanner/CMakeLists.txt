find_package(FLEX)

set(COMPILER_SCANNER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(COMPILER_SCANNER_LEXER ${COMPILER_SCANNER_DIR}/scanner.l)

if(FLEX_FOUND)
    flex_target(
        compiler_scanner_lexer
        ${COMPILER_SCANNER_LEXER}
        ${CMAKE_CURRENT_BINARY_DIR}/scanner_lex.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/scanner_lex.hpp
    )

    add_library(compiler_scanner STATIC)
    target_sources(
        compiler_scanner
        PRIVATE
            ${COMPILER_SCANNER_DIR}/scanner.cpp
            ${COMPILER_SCANNER_DIR}/scanner_runtime.cpp
            ${FLEX_compiler_scanner_lexer_OUTPUTS}
    )
    target_compile_definitions(compiler_scanner PUBLIC COMPILER_SCANNER_HAS_FLEX=1)
    target_include_directories(
        compiler_scanner
        PUBLIC
            ${PROJECT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_BINARY_DIR}
    )

    if(FLEX_INCLUDE_DIRS)
        target_include_directories(compiler_scanner PUBLIC ${FLEX_INCLUDE_DIRS})
    endif()
else()
    add_library(
        compiler_scanner
        STATIC
            ${COMPILER_SCANNER_DIR}/scanner.cpp
            ${COMPILER_SCANNER_DIR}/scanner_runtime.cpp
    )
    target_compile_definitions(compiler_scanner PUBLIC COMPILER_SCANNER_HAS_FLEX=0)
    target_include_directories(compiler_scanner PUBLIC ${PROJECT_SOURCE_DIR}/src)
endif()
