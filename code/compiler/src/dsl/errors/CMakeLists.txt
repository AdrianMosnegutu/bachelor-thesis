add_library(dsl_errors STATIC
        compiler_error.cpp
        lexical_error.cpp
        lowerer_error.cpp
        semantic_error.cpp
        syntax_error.cpp)
dsl_configure_library(dsl_errors)
target_link_libraries(dsl_errors PRIVATE dsl_location)
