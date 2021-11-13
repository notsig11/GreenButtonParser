cmake_minimum_required(VERSION 3.0)
project(greenbutton_reader)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(greenbutton_reader greenbutton.cpp)
