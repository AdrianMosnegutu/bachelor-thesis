include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2)
# Prevent GTest from installing itself alongside our project.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# -- Unit tests ------------------------------------------------------------
dsl_add_gtest(music_tests
        unit/common/music/note_test.cpp)
target_link_libraries(music_tests PRIVATE dsl_music)
target_include_directories(music_tests PRIVATE
        ${PROJECT_SOURCE_DIR}/src)

dsl_add_gtest(frontend_tests
        unit/frontend/scanner_test.cpp
        unit/frontend/parser_test.cpp)
target_link_libraries(frontend_tests PRIVATE dsl_frontend)
target_include_directories(frontend_tests PRIVATE
        ${CMAKE_BINARY_DIR}/src/dsl/frontend
        ${PROJECT_SOURCE_DIR}/src)

dsl_add_gtest(semantic_tests
        unit/semantic/semantic_analyzer_test.cpp)
target_link_libraries(semantic_tests PRIVATE dsl_frontend dsl_semantic)
target_include_directories(semantic_tests PRIVATE
        ${PROJECT_SOURCE_DIR}/src)

dsl_add_gtest(ir_tests
        unit/lowering/lowering_regression_test.cpp
        unit/lowering/lowerer_diagnostics_test.cpp
        unit/lowering/lowerer_test.cpp)
target_link_libraries(ir_tests PRIVATE dsl_frontend dsl_lowerer)
target_include_directories(ir_tests PRIVATE
        ${PROJECT_SOURCE_DIR}/src)

dsl_add_gtest(backend_tests
        unit/backend/midi_writer_test.cpp)
target_link_libraries(backend_tests PRIVATE dsl_backend)
target_include_directories(backend_tests PRIVATE
        ${PROJECT_SOURCE_DIR}/src)

# -- Integration tests -----------------------------------------------------
add_subdirectory(integration)

# -- Aggregate test targets ------------------------------------------------
add_custom_target(run_unit_tests
        COMMAND $<TARGET_FILE:music_tests>
        COMMAND $<TARGET_FILE:frontend_tests>
        COMMAND $<TARGET_FILE:semantic_tests>
        COMMAND $<TARGET_FILE:ir_tests>
        COMMAND $<TARGET_FILE:backend_tests>
        DEPENDS
            music_tests
            frontend_tests
            semantic_tests
            ir_tests
            backend_tests
        USES_TERMINAL
        VERBATIM)

add_custom_target(run_integration_unit_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS
            music_tests
            frontend_tests
            semantic_tests
            ir_tests
            backend_tests
            compiler_api_tests
            ir_integration_tests
            dslrc
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        USES_TERMINAL
        VERBATIM)

# -- Integration smoke test (existing) ------------------------------------
add_test(
        NAME parse_example
        COMMAND $<TARGET_FILE:dslrc> ${CMAKE_CURRENT_SOURCE_DIR}/data/example.dsl)
