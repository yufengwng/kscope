cmake_minimum_required(VERSION 3.1)

project(kscope VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(kscope src/main.cpp)
target_include_directories(kscope PUBLIC src)
