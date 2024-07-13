cmake_minimum_required(VERSION 3.20)

project(BoardGameStats VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC")  # full path in error messages

file(GLOB SOURCES "*.cpp")

add_executable(BoardGameStats ${SOURCES})
