add_library(dsl_ir STATIC)

add_subdirectory(expression_evaluators)
add_subdirectory(value_flatteners)
add_subdirectory(ast_lowerers)

target_sources(dsl_ir PRIVATE
        expression_evaluator.cpp
        value_flattener.cpp
        lowerer_context.cpp
        lowerer.cpp)

dsl_configure_library(dsl_ir)
target_link_libraries(dsl_ir
        PRIVATE dsl_errors dsl_music dsl_semantic)
