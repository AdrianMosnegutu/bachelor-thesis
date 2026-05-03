add_subdirectory(dsl/common)
add_subdirectory(dsl/parsing)
add_subdirectory(dsl/semantic)
add_subdirectory(dsl/lowering)
add_subdirectory(dsl/midi)

add_library(dsl_compiler STATIC dsl/compiler.cpp)
dsl_configure_public_library(dsl_compiler)
target_link_libraries(dsl_compiler
        PRIVATE dsl_diagnostics dsl_parsing dsl_semantic dsl_lowering dsl_midi)
