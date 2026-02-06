cmake_minimum_required(VERSION 3.20)
project(FloatDoubleArray)

option(DOUBLE "Using double" OFF)

add_executable(main main.cpp)

if(DOUBLE)
    target_compile_definitions(main PRIVATE DOUBLE)
endif()