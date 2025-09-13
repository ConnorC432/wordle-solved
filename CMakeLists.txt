cmake_minimum_required(VERSION 3.22.1)
project(wordle_solved VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Release flags: highest optimization, LTO, static standard library
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -march=x86-64 -mtune=generic -fno-exceptions -fno-rtti -DNDEBUG -static-libstdc++ -static-libgcc")

    # Debug flags: optimization + debug symbols
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra -pedantic -fno-omit-frame-pointer")

    # RelWithDebInfo: optimized + debug symbols
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O0 -g -flto -march=x86-64 -mtune=generic -DNDEBUG -static-libstdc++ -static-libgcc")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(wordle_solved
        wordle.cpp
        Display.cpp Display.h
        Feedback.cpp Feedback.h
        Entropy.cpp Entropy.h
        InteractiveMode.cpp InteractiveMode.h
        AutoMode.cpp AutoMode.h
        ThreadPool.h)

include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
    set_target_properties(wordle_solved PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()