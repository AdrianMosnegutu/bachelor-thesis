add_executable(integration_tests
        compiler_integration_test.cpp
)

target_link_libraries(integration_tests PRIVATE dsl_frontend dsl_ir GTest::gtest_main)

target_include_directories(integration_tests PRIVATE
        ${CMAKE_BINARY_DIR}/src/frontend)

target_compile_definitions(integration_tests PRIVATE
        INTEGRATION_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

include(GoogleTest)
gtest_discover_tests(integration_tests)
