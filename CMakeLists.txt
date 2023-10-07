cmake_minimum_required(VERSION 3.26)
project(pa_cpp)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(CMAKE_CXX_STANDARD 14)

add_executable(pa_cpp main-thread.cpp)
add_executable(pa_2 pa2.cpp
        pa2.cpp)
