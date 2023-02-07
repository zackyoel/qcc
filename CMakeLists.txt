cmake_minimum_required(VERSION 3.0.0)
project(qcc VERSION 0.1.0)


file(GLOB SRC_FILE
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.c"
)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILE})

# 编译参数
target_compile_options(qcc PRIVATE -std=c11 -g -fno-common)

