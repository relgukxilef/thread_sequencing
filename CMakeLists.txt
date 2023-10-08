cmake_minimum_required(VERSION 3.5)

project(thread_sequencing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    tests main.cpp
    include/sequence/sequence.h
    source/sequence.cpp
    include/sequence/thread.h
    include/sequence/atomic.h
)

target_include_directories(tests PUBLIC include)

target_compile_features(tests PUBLIC cxx_std_20)

enable_testing()
add_test(NAME tests COMMAND tests)
