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
    message(STATUS "SDL3 not found, downloading from GitHub")
    
    include(FetchContent)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG ${SDL_VERSION}
        GIT_SHALLOW TRUE
    )
    
    FetchContent_MakeAvailable(SDL3)
    
    message(STATUS "SDL3 downloaded and configured")
endif()

# Create our own wrapper target
add_library(SDL INTERFACE)
target_link_libraries(SDL INTERFACE SDL3::SDL3)