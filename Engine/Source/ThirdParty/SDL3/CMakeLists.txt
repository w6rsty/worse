find_package(SDL3 QUIET)

set(SDL_VERSION release-3.2.14)

if (SDL3_FOUND)
    message(STATUS "Found SDL3 in system")
    # Create an interface target to normalize the usage
    if(NOT TARGET SDL3::SDL3)
        add_library(SDL3::SDL3 INTERFACE IMPORTED)
        set_target_properties(SDL3::SDL3 PROPERTIES
            INTERFACE_LINK_LIBRARIES SDL3
        )
        # Find and add include directories
        find_path(SDL3_INCLUDE_DIRS
            NAMES SDL3/SDL.h
            PATH_SUFFIXES include
            DOC "SDL3 include directory"
        )
        target_include_directories(SDL3::SDL3 INTERFACE ${SDL3_INCLUDE_DIRS})
        message(STATUS "SDL3 include directory: ${SDL3_INCLUDE_DIRS}")
    endif()
else()
    message(STATUS "SDL3 not found, using local copy")
    
    # Use local SDL copy
    set(SDL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../ThirdParty/SDL)
    
    if(NOT EXISTS ${SDL_ROOT})
        message(FATAL_ERROR "SDL not found at ${SDL_ROOT}. Please run download_dependencies.sh first.")
    endif()

    set(SDL_SHARED OFF CACHE BOOL "Build shared library" FORCE)
    set(SDL_STATIC ON CACHE BOOL "Build static library" FORCE)
    set(SDL_TEST OFF CACHE BOOL "Build test programs" FORCE)
    set(SDL_DISABLE_INSTALL ON CACHE BOOL "Disable installation" FORCE)
    set(SDL_ASSERTIONS disabled CACHE STRING "Enable internal sanity checks" FORCE)

    add_subdirectory(${SDL_ROOT} ${CMAKE_BINARY_DIR}/sdl EXCLUDE_FROM_ALL)
endif()

# Create our own wrapper target
add_library(SDL INTERFACE)
target_link_libraries(SDL INTERFACE SDL3::SDL3)