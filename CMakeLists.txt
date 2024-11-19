cmake_minimum_required(VERSION 3.12)

project(Compiler VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

file(GLOB_RECURSE LIB_SOURCES "lib/*.cpp")

add_library(CompilerLib STATIC ${LIB_SOURCES})

add_executable(Compiler main.cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_link_libraries(Compiler PRIVATE CompilerLib)