cmake_minimum_required(VERSION 3.15)
project(DataStructures
    VERSION 1.0.0
    DESCRIPTION "High-performance modern C++ implementations of classic data structures"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

# Header-only library interface
add_library(search_trees INTERFACE)
target_include_directories(search_trees INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Interactive Demo Executable
add_executable(demo src/main.cpp)
target_link_libraries(demo PRIVATE search_trees)

# Testing
enable_testing()

add_executable(test-avl-tree tests/test-avl-tree.cpp)
target_link_libraries(test-avl-tree PRIVATE search_trees)
add_test(NAME AvlTreeTest COMMAND test-avl-tree)

add_executable(test-red-black-tree tests/test-red-black-tree.cpp)
target_link_libraries(test-red-black-tree PRIVATE search_trees)
add_test(NAME RedBlackTreeTest COMMAND test-red-black-tree)

add_executable(test-b-tree tests/test-b-tree.cpp)
target_link_libraries(test-b-tree PRIVATE search_trees)
add_test(NAME BTreeTest COMMAND test-b-tree)

add_executable(test-runner tests/test-runner.cpp)
target_link_libraries(test-runner PRIVATE search_trees)
add_test(NAME UnifiedTestRunner COMMAND test-runner)

# Performance Benchmark
add_executable(benchmark tests/benchmark.cpp)
target_link_libraries(benchmark PRIVATE search_trees)
