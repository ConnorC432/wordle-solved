cmake_minimum_required(VERSION 4.0)
project(wordle_solved)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static-libstdc++ -static-libgcc")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2 -g -pg")

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g -DNDEBUG -static-libstdc++ -static-libgcc")

add_executable(wordle_solved
        wordle.cpp
        Display.cpp
        Display.h
        Feedback.cpp
        Feedback.h
        Entropy.cpp
        Entropy.h
        InteractiveMode.cpp
        InteractiveMode.h
        AutoMode.cpp
        AutoMode.h
        ThreadPool.h)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
endif()