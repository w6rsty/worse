set(DXC_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(DXC_INCLUDE_DIR ${DXC_ROOT_DIR}/include)
set(DXC_LIB_DIR ${DXC_ROOT_DIR}/lib)

# Search in system paths first, use binary in this directory if failed
find_library(DXC_LIBRARY
    NAMES dxcompiler libdxcompiler
    PATHS ${DXC_LIB_DIR}
    NO_DEFAULT_PATH
)

if(NOT DXC_LIBRARY)
    message(FATAL_ERROR "DXC library not found in ${DXC_LIB_DIR}")
endif()

message(STATUS "Found DXC library: ${DXC_LIBRARY}")

add_library(DXC INTERFACE)

target_include_directories(DXC INTERFACE
    ${DXC_INCLUDE_DIR}
)

target_link_libraries(DXC INTERFACE
    ${DXC_LIBRARY}
)