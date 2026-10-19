cmake_minimum_required(VERSION 3.20)
project(memory_management LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(memory_management
    main.cpp
    examples/Student.cpp
    examples/01_new_delete.cpp
    examples/02_array.cpp
    examples/03_student.cpp
    examples/04_malloc_free.cpp
    examples/05_placement_new.cpp
)

if(MSVC)
    target_compile_options(memory_management PRIVATE /W4 /utf-8)
else()
    target_compile_options(memory_management PRIVATE -Wall -Wextra -Wpedantic)
endif()
