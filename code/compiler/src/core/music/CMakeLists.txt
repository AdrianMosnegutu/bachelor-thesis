add_library(dsl_music STATIC accidental.cpp note.cpp pitch.cpp)
target_include_directories(dsl_music PUBLIC ${PROJECT_SOURCE_DIR}/include)
