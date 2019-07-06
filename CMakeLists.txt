cmake_minimum_required(VERSION 3.5.2)
project(RISC_V)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2  ")

add_executable(code
#        test.cpp
        main.cpp
        basic.hpp
        )