cmake_minimum_required(VERSION 3.10)

project(udisruptor)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(benchmark
  benchmark.cpp
)

target_include_directories(benchmark PUBLIC
    "${PROJECT_SOURCE_DIR}/../include"
    "${PROJECT_SOURCE_DIR}/../thirdparty/include"
)
