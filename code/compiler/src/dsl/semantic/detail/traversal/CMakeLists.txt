add_subdirectory(visitors)

target_sources(dsl_semantic PRIVATE
        collectors.cpp
        validators.cpp
)
