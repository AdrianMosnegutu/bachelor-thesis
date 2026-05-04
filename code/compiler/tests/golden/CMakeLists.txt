dsl_add_gtest(golden_tests golden_test.cpp)
target_link_libraries(golden_tests PRIVATE dsl_compiler)
target_compile_definitions(golden_tests PRIVATE
        GOLDEN_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
