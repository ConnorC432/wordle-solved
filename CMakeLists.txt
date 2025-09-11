cmake_minimum_required(VERSION 4.0)
project(wordle_solved)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(wordle_solved
        wordle.cpp
        words.h)
