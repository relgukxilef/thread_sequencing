cmake_minimum_required(VERSION 3.5)

project(thread_sequencing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    tests main.cpp
    include/sequence/sequence.h
    source/sequence.cpp
    include/sequence/thread.h
)

target_include_directories(tests PUBLIC include)

enable_testing()
add_test(NAME tests COMMAND tests)
