add_library(dsl_semantic STATIC)

add_subdirectory(detail)

target_sources(dsl_semantic PRIVATE
        analysis_result.cpp
        analyze.cpp
)

dsl_configure_library(dsl_semantic)
target_link_libraries(dsl_semantic PUBLIC dsl_diagnostics PRIVATE dsl_location)
