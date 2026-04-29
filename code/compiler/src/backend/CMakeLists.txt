add_library(dsl_backend STATIC midi_writer.cpp)
target_include_directories(dsl_backend PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(dsl_backend PUBLIC dsl_ir)
