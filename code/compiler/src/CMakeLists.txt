add_library(dsl_location STATIC dsl/source/location.cpp dsl/source/position.cpp)
dsl_configure_library(dsl_location)

add_subdirectory(dsl/diagnostics)
add_subdirectory(dsl/music)
add_subdirectory(dsl/frontend)
add_subdirectory(dsl/semantic)
add_subdirectory(dsl/lowerer)
add_subdirectory(dsl/backend)

add_library(dsl_compiler STATIC dsl/compiler.cpp)
dsl_configure_public_library(dsl_compiler)
target_link_libraries(dsl_compiler
        PRIVATE dsl_diagnostics dsl_frontend dsl_semantic dsl_lowerer dsl_backend)
