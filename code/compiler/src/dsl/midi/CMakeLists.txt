add_library(dsl_midi STATIC write_midi.cpp)
dsl_configure_library(dsl_midi)
target_link_libraries(dsl_midi PRIVATE dsl_lowering)
