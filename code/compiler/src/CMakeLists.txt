set(COMPILER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE COMPILER_APP_SOURCES CONFIGURE_DEPENDS ${COMPILER_SRC_DIR}/*.cpp)
file(GLOB_RECURSE COMPILER_APP_HEADERS CONFIGURE_DEPENDS ${COMPILER_SRC_DIR}/*.h ${COMPILER_SRC_DIR}/*.hpp)

set(COMPILER_MAIN_SOURCE ${COMPILER_SRC_DIR}/main.cpp)
list(REMOVE_ITEM COMPILER_APP_SOURCES ${COMPILER_MAIN_SOURCE})
list(FILTER COMPILER_APP_SOURCES EXCLUDE REGEX "^${COMPILER_SRC_DIR}/scanner/")
list(FILTER COMPILER_APP_HEADERS EXCLUDE REGEX "^${COMPILER_SRC_DIR}/scanner/")

add_subdirectory(scanner)

if(COMPILER_APP_SOURCES)
    add_library(compiler_lib)
    target_sources(
        compiler_lib
        PRIVATE
            ${COMPILER_APP_SOURCES}
        PUBLIC
            FILE_SET HEADERS
            BASE_DIRS ${COMPILER_SRC_DIR}
            FILES ${COMPILER_APP_HEADERS}
    )
    target_include_directories(compiler_lib PUBLIC ${COMPILER_SRC_DIR})
    target_link_libraries(compiler_lib PUBLIC compiler_scanner)
else()
    add_library(compiler_lib INTERFACE)
    target_include_directories(compiler_lib INTERFACE ${COMPILER_SRC_DIR})
    target_link_libraries(compiler_lib INTERFACE compiler_scanner)
endif()

add_executable(compiler ${COMPILER_MAIN_SOURCE})
target_link_libraries(compiler PRIVATE compiler_lib)
