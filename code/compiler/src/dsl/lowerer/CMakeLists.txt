add_library(dsl_lowerer STATIC)

add_subdirectory(detail)

target_sources(dsl_lowerer PRIVATE
        lowerer.cpp)

dsl_configure_library(dsl_lowerer)
target_link_libraries(dsl_lowerer
        PUBLIC dsl_diagnostics
        PRIVATE dsl_music dsl_semantic)
