include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2)
# Prevent GTest from installing itself alongside our project.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# -- Shared include paths -------------------------------------------------------

set(DSL_SRC ${PROJECT_SOURCE_DIR}/src)
set(DSL_BINARY_PARSING ${CMAKE_BINARY_DIR}/src/dsl/parsing)

# -- Common module unit tests ---------------------------------------------------

dsl_add_gtest(common_tests
        unit/common/music/note_test.cpp)
target_link_libraries(common_tests PRIVATE dsl_music)
target_include_directories(common_tests PRIVATE ${DSL_SRC})

# -- Parsing unit tests ---------------------------------------------------------

file(GLOB_RECURSE PARSING_SOURCES
        unit/parsing/*.cpp)

dsl_add_gtest(parsing_tests ${PARSING_SOURCES})
target_link_libraries(parsing_tests PRIVATE dsl_parsing)
target_include_directories(parsing_tests PRIVATE
        ${DSL_BINARY_PARSING}
        ${DSL_SRC}
        unit/parsing)

# -- Semantic unit tests --------------------------------------------------------

file(GLOB_RECURSE SEMANTIC_SOURCES
        unit/semantic/scope_access_test.cpp
        unit/semantic/metadata_test.cpp
        unit/semantic/type_checking/*.cpp
        unit/semantic/scoping/*.cpp
        unit/semantic/declarations/*.cpp
        unit/semantic/statements/*.cpp)

dsl_add_gtest(semantic_tests ${SEMANTIC_SOURCES})
target_link_libraries(semantic_tests PRIVATE dsl_parsing dsl_semantic)
target_include_directories(semantic_tests PRIVATE
        ${DSL_SRC}
        unit/semantic)

# -- Lowering unit tests --------------------------------------------------------

file(GLOB_RECURSE LOWERING_SOURCES
        unit/lowering/cursor/*.cpp
        unit/lowering/control_flow/*.cpp
        unit/lowering/diagnostics/*.cpp
        unit/lowering/expressions/*.cpp
        unit/lowering/flattening/*.cpp
        unit/lowering/patterns/*.cpp
        unit/lowering/voices/*.cpp
        unit/lowering/variables/*.cpp)

dsl_add_gtest(ir_tests ${LOWERING_SOURCES})
target_link_libraries(ir_tests PRIVATE dsl_parsing dsl_lowering)
target_include_directories(ir_tests PRIVATE
        ${DSL_SRC}
        unit/lowering)

# -- MIDI unit tests ------------------------------------------------------------

dsl_add_gtest(midi_tests
        unit/midi/midi_writer_test.cpp)
target_link_libraries(midi_tests PRIVATE dsl_midi)
target_include_directories(midi_tests PRIVATE ${DSL_SRC})

# -- Integration tests ----------------------------------------------------------

add_subdirectory(integration)

# -- Golden end-to-end tests ----------------------------------------------------

add_subdirectory(golden)

# -- Aggregate targets ----------------------------------------------------------

add_custom_target(run_unit_tests
        COMMAND $<TARGET_FILE:common_tests>
        COMMAND $<TARGET_FILE:parsing_tests>
        COMMAND $<TARGET_FILE:semantic_tests>
        COMMAND $<TARGET_FILE:ir_tests>
        COMMAND $<TARGET_FILE:midi_tests>
        DEPENDS
            common_tests
            parsing_tests
            semantic_tests
            ir_tests
            midi_tests
        USES_TERMINAL
        VERBATIM)

add_custom_target(run_integration_unit_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS
            common_tests
            parsing_tests
            semantic_tests
            ir_tests
            midi_tests
            compiler_api_tests
            golden_tests
            dslrc
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        USES_TERMINAL
        VERBATIM)
