add_subdirectory(definition_lowerers)
add_subdirectory(statement_lowerers)

target_sources(dsl_ir PRIVATE
        lower_header.cpp
        lower_block.cpp
        lower_statement.cpp
)
