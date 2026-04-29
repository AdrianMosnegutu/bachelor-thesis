add_executable(dslrc dslrc.cpp)
target_link_libraries(dslrc PRIVATE dsl_frontend dsl_ir dsl_backend)
