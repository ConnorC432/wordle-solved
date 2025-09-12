cmake_minimum_required(VERSION 4.0)
project(wordle_solved)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static-libstdc++ -static-libgcc")

add_executable(wordle_solved
        wordle.cpp
        words.h)
