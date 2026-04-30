function(dsl_configure_library target)
    target_include_directories(${target}
            PRIVATE ${PROJECT_SOURCE_DIR}/src)
endfunction()

function(dsl_configure_public_library target)
    target_include_directories(${target}
            PUBLIC  ${PROJECT_SOURCE_DIR}/include
            PRIVATE ${PROJECT_SOURCE_DIR}/src)
endfunction()

function(dsl_add_gtest target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE GTest::gtest_main)
    gtest_discover_tests(${target})
endfunction()
