cmake_minimum_required(VERSION 3.20)
project(CPPString LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)          # 用 -std=c++20，而不是 -std=gnu++20

# 默认打开常用警告，编译期就能发现越界、未使用变量、隐式类型转换之类的问题
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 只把类的实现编成静态库。main.cpp 属于可执行文件，不能放进库里：
# 库里的 main 不会被调用，将来加测试程序时还会造成重定义
add_library(stringlib STATIC
        string/string.cpp
        string/string.h
)
# 对外暴露的包含目录是项目根，使用者写 #include "string/string.h"。
# 注意不要把 string/ 本身加进来：里面的 string.h 会顶掉 C 标准库的 <string.h>
target_include_directories(stringlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(CPPString main.cpp)
target_link_libraries(CPPString PRIVATE stringlib)
