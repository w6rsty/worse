# Open3D precompiled library integration
cmake_minimum_required(VERSION 3.12)

# Set the path to the Open3D installation
set(Open3D_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Add the Open3D cmake modules to the path
list(APPEND CMAKE_PREFIX_PATH ${Open3D_ROOT})

# Find Open3D package
find_package(Open3D REQUIRED PATHS ${Open3D_ROOT}/lib/cmake/Open3D NO_DEFAULT_PATH)

if(Open3D_FOUND)
    message(STATUS "Found Open3D ${Open3D_VERSION}")
    
    # Create an interface target for Open3D
    if(NOT TARGET Open3D::Open3D)
        add_library(Open3D::Open3D INTERFACE IMPORTED)
        
        # Set include directories
        target_include_directories(Open3D::Open3D INTERFACE
            ${Open3D_ROOT}/include
        )
        
        # Set library path
        target_link_directories(Open3D::Open3D INTERFACE
            ${Open3D_ROOT}/lib
        )
        
        # Link the main Open3D library
        target_link_libraries(Open3D::Open3D INTERFACE
            ${Open3D_ROOT}/lib/libOpen3D.dylib
        )
        
        # On macOS, we might need to set RPATH
        if(APPLE)
            set_target_properties(Open3D::Open3D PROPERTIES
                INTERFACE_LINK_OPTIONS "-Wl,-rpath,${Open3D_ROOT}/lib"
            )
        endif()
    endif()
    
    # Export the target for use by other parts of the project
    set(Open3D_LIBRARIES Open3D::Open3D PARENT_SCOPE)
    set(Open3D_INCLUDE_DIRS ${Open3D_ROOT}/include PARENT_SCOPE)
    
else()
    message(FATAL_ERROR "Open3D not found in ${Open3D_ROOT}")
endif()

# Print useful information
message(STATUS "Open3D include directory: ${Open3D_ROOT}/include")
message(STATUS "Open3D library directory: ${Open3D_ROOT}/lib")
