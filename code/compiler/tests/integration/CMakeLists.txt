dsl_add_gtest(compiler_api_tests
        compiler_api_test.cpp)
target_link_libraries(compiler_api_tests PRIVATE dsl_compiler)

dsl_add_gtest(ir_integration_tests
        compiler_integration_test.cpp
)

target_link_libraries(ir_integration_tests PRIVATE dsl_frontend dsl_ir)

target_include_directories(ir_integration_tests PRIVATE
        ${PROJECT_SOURCE_DIR}/src)

target_compile_definitions(ir_integration_tests PRIVATE
        INTEGRATION_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
