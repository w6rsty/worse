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
    message(STATUS "SDL3 not found, fetch from https://github.com/libsdl-org/SDL.git, tag ${SDL_VERSION}")
endif()

if(NOT SDL3_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG ${SDL_VERSION}
    )

    set(SDL_SHARED OFF CACHE BOOL "Build shared library" FORCE)
    set(SDL_STATIC ON CACHE BOOL "Build static library" FORCE)
    set(SDL_TEST OFF CACHE BOOL "Build test programs" FORCE)
    set(SDL_DISABLE_INSTALL ON CACHE BOOL "Disable installation" FORCE)
    set(SDL_ASSERTIONS disabled CACHE STRING "Enable internal sanity checks" FORCE)

    FetchContent_MakeAvailable(SDL3)
endif()

# Create our own wrapper target
add_library(SDL INTERFACE)
target_link_libraries(SDL INTERFACE SDL3::SDL3)