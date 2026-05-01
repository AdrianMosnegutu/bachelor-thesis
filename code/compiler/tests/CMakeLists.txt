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
        unit/core/music/note_test.cpp)
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

# -- Integration smoke test (existing) ------------------------------------
add_test(
        NAME parse_example
        COMMAND $<TARGET_FILE:dslrc> ${CMAKE_CURRENT_SOURCE_DIR}/data/example.dsl)
