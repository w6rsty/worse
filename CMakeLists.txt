cmake_minimum_required(VERSION 3.20)
project(worse VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

# Generate clangd database (When using Ninja and Makefile)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Make sure clangd can find compile_commands.json
if(PROJECT_IS_TOP_LEVEL)
    if(UNIX)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink
                    ${CMAKE_BINARY_DIR}/compile_commands.json
                    ${CMAKE_SOURCE_DIR}/compile_commands.json
            RESULT_VARIABLE result
        )
        if(NOT result EQUAL 0)
        endif()
    elseif(WIN32)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_BINARY_DIR}/compile_commands.json
                    ${CMAKE_SOURCE_DIR}/compile_commands.json
        )
    endif()
endif()

# Download directory for third-party libraries
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR})

add_subdirectory(Engine/Source)