add_subdirectory(traversal)
add_subdirectory(scopes)
add_subdirectory(types)

target_sources(dsl_semantic PRIVATE
        traversal.cpp
)
