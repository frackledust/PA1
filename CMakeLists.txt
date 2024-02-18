cmake_minimum_required(VERSION 3.26)
project(pa_cpp)

set(CMAKE_CXX_STANDARD 14)

add_executable(exercise exercise.cpp)
add_executable(project_1 project_1.cpp)
add_executable(project_2 project_2.cpp)
add_executable(project_3 project_3.cpp)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    target_link_libraries(project_2 PRIVATE OpenMP::OpenMP_CXX)
    target_link_libraries(project_3 PRIVATE OpenMP::OpenMP_CXX)
endif()
