add_library(dsl_semantic STATIC)

add_subdirectory(detail)

target_sources(dsl_semantic PRIVATE
        analysis_result.cpp
        analyzer.cpp
        annotations.cpp
        symbol_table.cpp
)

target_include_directories(dsl_semantic PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(dsl_semantic PUBLIC dsl_frontend)
