cmake_minimum_required(VERSION 3.20)
project(worse VERSION 0.2.0 LANGUAGES CXX)

set(WS_ENGINE_DIR "${CMAKE_SOURCE_DIR}")
add_compile_definitions(WS_ENGINE_DIR="${WS_ENGINE_DIR}")

add_subdirectory(Source)
