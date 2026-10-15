cmake_minimum_required(VERSION 4.0)
project(Outset)

set(CMAKE_CXX_STANDARD 20)

add_executable(Outset main.cpp
        Outset.cpp
        Outset.h)

# Debug 模式下开启 -O2 优化，同时保留调试信息
target_compile_options(Outset PRIVATE
        $<$<CONFIG:Debug>:-O2>
)