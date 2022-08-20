cmake_minimum_required(VERSION 3.23)
project(program_1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

add_executable(program_1 main.cpp)
