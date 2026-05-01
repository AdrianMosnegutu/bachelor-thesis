add_library(dsl_diagnostics STATIC
        diagnostics_engine.cpp)
dsl_configure_public_library(dsl_diagnostics)
target_link_libraries(dsl_diagnostics PRIVATE dsl_location)
