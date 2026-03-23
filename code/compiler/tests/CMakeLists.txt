set(COMPILER_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(
    COMPILER_TEST_SOURCES
    ${COMPILER_TESTS_DIR}/scanner_test.cpp
)

if(COMPILER_GTEST_SOURCE_DIR)
    add_subdirectory(${COMPILER_GTEST_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
else()
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

if(COMPILER_TEST_SOURCES)
    if(TARGET GTest::gtest_main)
        set(COMPILER_GTEST_MAIN_TARGET GTest::gtest_main)
    elseif(TARGET gtest_main)
        set(COMPILER_GTEST_MAIN_TARGET gtest_main)
    else()
        message(FATAL_ERROR "GoogleTest was added, but neither GTest::gtest_main nor gtest_main is available.")
    endif()

    add_executable(compiler_tests ${COMPILER_TEST_SOURCES})
    target_link_libraries(compiler_tests PRIVATE compiler_lib ${COMPILER_GTEST_MAIN_TARGET})
    target_include_directories(compiler_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_compile_definitions(compiler_tests PRIVATE COMPILER_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

    include(GoogleTest)
    gtest_discover_tests(compiler_tests)
endif()
