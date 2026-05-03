add_subdirectory(traversal)
add_subdirectory(scopes)
add_subdirectory(types)

target_sources(dsl_semantic PRIVATE
        annotations.cpp
        symbol_table.cpp
        traversal.cpp
)
