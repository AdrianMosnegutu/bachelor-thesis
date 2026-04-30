add_subdirectory(value_flatteners)
add_subdirectory(ast_lowerers)

target_sources(dsl_ir PRIVATE
        value_flattener.cpp
        lowerer_context.cpp
)
