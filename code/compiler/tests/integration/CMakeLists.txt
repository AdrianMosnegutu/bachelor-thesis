dsl_add_gtest(compiler_api_tests
        compiler_api_test.cpp)
target_link_libraries(compiler_api_tests PRIVATE dsl_compiler)
