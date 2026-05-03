add_library(dsl_lowering STATIC)

add_subdirectory(detail)

target_sources(dsl_lowering PRIVATE
        lower_result.cpp
        lower.cpp)

dsl_configure_library(dsl_lowering)
target_link_libraries(dsl_lowering
        PUBLIC dsl_diagnostics
        PRIVATE dsl_music dsl_semantic)
