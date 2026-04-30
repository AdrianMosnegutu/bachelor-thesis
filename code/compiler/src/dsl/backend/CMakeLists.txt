add_library(dsl_backend STATIC midi_writer.cpp)
dsl_configure_library(dsl_backend)
target_link_libraries(dsl_backend PRIVATE dsl_ir)
