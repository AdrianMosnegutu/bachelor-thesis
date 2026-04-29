add_subdirectory(evaluators)
add_subdirectory(flatteners)
add_subdirectory(lowerers)

add_library(dsl_ir STATIC
    ${IR_EVALUATOR_SOURCES}
    ${IR_FLATTENER_SOURCES}
    ${IR_LOWERER_SOURCES}
    lowerer.cpp
    expression_evaluator.cpp
    value_flattener.cpp
)
target_include_directories(dsl_ir PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(dsl_ir PUBLIC dsl_frontend)
