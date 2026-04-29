include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2)
# Prevent GTest from installing itself alongside our project.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# -- Unit tests ------------------------------------------------------------
add_executable(unit_tests
        unit/music/note_test.cpp
        unit/frontend/scanner_test.cpp
        unit/frontend/parser_test.cpp
        unit/ir/lowerer_test.cpp
        unit/backend/midi_writer_test.cpp
)
target_link_libraries(unit_tests PRIVATE dsl_frontend dsl_ir dsl_backend GTest::gtest_main)
# The build dir of dsl_frontend holds the generated parser.hpp / scanner.
target_include_directories(unit_tests PRIVATE
        ${CMAKE_BINARY_DIR}/src/frontend)

include(GoogleTest)
gtest_discover_tests(unit_tests)

# -- Integration tests -----------------------------------------------------
add_subdirectory(integration)

# -- Integration smoke test (existing) ------------------------------------
add_test(
        NAME parse_example
        COMMAND $<TARGET_FILE:dslrc> ${CMAKE_CURRENT_SOURCE_DIR}/data/example.dsl)
