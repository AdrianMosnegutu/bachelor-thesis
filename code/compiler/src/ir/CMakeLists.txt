add_library(dsl_ir STATIC)

add_subdirectory(detail)

target_sources(dsl_ir PUBLIC lowerer.cpp)

target_include_directories(dsl_ir PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(dsl_ir PUBLIC dsl_frontend)
