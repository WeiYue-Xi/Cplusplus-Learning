cmake_minimum_required(VERSION 3.20)
project(CPPString)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(CPPString main.cpp
        string.cpp
        string.h
)
