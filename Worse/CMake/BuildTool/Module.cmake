include_guard()
include(${CMAKE_SOURCE_DIR}/CMake/Config.cmake)

macro(__OutOfModuleEarlyReturn)
    if (NOT DEFINED __CURRENT_MODULE_NAME)
        set(__caller ${CMAKE_CURRENT_FUNCTION})
        message(WARNING "${__caller} outside of module declaration")
        return()
    endif()
endmacro()


macro(PrivateDependencyModuleAdd)
    __OutOfModuleEarlyReturn()

    foreach(dep_module ${ARGN})
        list(APPEND __${__CURRENT_MODULE_NAME}_PRIVATE_DEPENDENCIES "${dep_module}")
    endforeach()
endmacro()

macro(PublicDependencyModuleAdd)
    __OutOfModuleEarlyReturn()

    foreach(dep_module ${ARGN})
        list(APPEND __${__CURRENT_MODULE_NAME}_PUBLIC_DEPENDENCIES "${dep_module}")
    endforeach()
endmacro()


macro(PublicDefinitions)
    __OutOfModuleEarlyReturn()

    foreach(def ${ARGN})
        list(APPEND __${__CURRENT_MODULE_NAME}_PUBLIC_DEFINITIONS "${def}")
    endforeach()
endmacro()

macro(PrivateDefinitions)
    __OutOfModuleEarlyReturn()

    foreach(def ${ARGN})
        list(APPEND __${__CURRENT_MODULE_NAME}_PRIVATE_DEFINITIONS "${def}")
    endforeach()
endmacro()


macro(PublicPch HEADER)
    set(__${__CURRENT_MODULE_NAME}_PUBLIC_PCH "${HEADER}")
endmacro()

macro(PrivatePch HEADER)
    set(__${__CURRENT_MODULE_NAME}_PRIVATE_PCH "${HEADER}")
endmacro()


macro(ModuleCppStandard STANDARD)
    set(__${__CURRENT_MODULE_NAME}_CPP_STANDARD "${STANDARD}")
endmacro()


function(__CollectConfigs)
    __OutOfModuleEarlyReturn()

    # Set defaults
    if(NOT DEFINED __${__CURRENT_MODULE_NAME}_PUBLIC_DEPENDENCIES)
        set(__${__CURRENT_MODULE_NAME}_PUBLIC_DEPENDENCIES "" PARENT_SCOPE)
    endif()
    if(NOT DEFINED __${__CURRENT_MODULE_NAME}_PRIVATE_DEPENDENCIES)
        set(__${__CURRENT_MODULE_NAME}_PRIVATE_DEPENDENCIES "" PARENT_SCOPE)
    endif()
    if(NOT DEFINED __${__CURRENT_MODULE_NAME}_PUBLIC_DEFINITIONS)
        set(__${__CURRENT_MODULE_NAME}_PUBLIC_DEFINITIONS "" PARENT_SCOPE)
    endif()
    if(NOT DEFINED __${__CURRENT_MODULE_NAME}_PRIVATE_DEFINITIONS)
        set(__${__CURRENT_MODULE_NAME}_PRIVATE_DEFINITIONS "" PARENT_SCOPE)
    endif()
    if(NOT DEFINED __${__CURRENT_MODULE_NAME}_CPP_STANDARD)
        set(__${__CURRENT_MODULE_NAME}_CPP_STANDARD ${WORSE_DEFAULT_CXX_STANDARD} PARENT_SCOPE)
    endif()

    # Register properties

    set_target_properties(${__CURRENT_MODULE_NAME} PROPERTIES
        PUBLIC_DEPENDENCIES
            "${__${__CURRENT_MODULE_NAME}_PUBLIC_DEPENDENCIES}"
        PRIVATE_DEPENDENCIES
            "${__${__CURRENT_MODULE_NAME}_PRIVATE_DEPENDENCIES}"
        PUBLIC_DEFINITIONS
            "${__${__CURRENT_MODULE_NAME}_PUBLIC_DEFINITIONS}"
        PRIVATE_DEFINITIONS
            "${__${__CURRENT_MODULE_NAME}_PRIVATE_DEFINITIONS}"
        CXX_STANDARD
            "${__${__CURRENT_MODULE_NAME}_CPP_STANDARD}")
endfunction()

function(DeclareModule MODULE_NAME)
    set(__CURRENT_MODULE_NAME "${MODULE_NAME}" PARENT_SCOPE)
endfunction()

function(EndDeclareModule)
    __OutOfModuleEarlyReturn()
    
    add_library(${__CURRENT_MODULE_NAME} STATIC)

    __CollectConfigs()

    set_target_properties(${__CURRENT_MODULE_NAME} PROPERTIES
        CXX_STANDARD "${__${__CURRENT_MODULE_NAME}_CPP_STANDARD}")

    file(GLOB_RECURSE MODULE_PUBLIC_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/public/*.hpp)
    file(GLOB_RECURSE MODULE_PRIVATE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/private/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/private/*.cpp)
    
    source_group("public" FILES ${MODULE_PUBLIC_SOURCES})
    source_group("private" FILES ${MODULE_PRIVATE_SOURCES})

    target_sources(${__CURRENT_MODULE_NAME}
        PRIVATE ${MODULE_PRIVATE_SOURCES}
        PUBLIC ${MODULE_PUBLIC_SOURCES})

    target_include_directories(${__CURRENT_MODULE_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
        PRIVATE 
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private>)

    if (DEFINED __${MODULE_NAME}_PUBLIC_PCH)
        target_precompile_headers(${__CURRENT_MODULE_NAME} PUBLIC ${__${MODULE_NAME}_PUBLIC_PCH})
    endif()
    if (DEFINED __${MODULE_NAME}_PRIVATE_PCH)
        target_precompile_headers(${__CURRENT_MODULE_NAME} PRIVATE ${__${MODULE_NAME}_PRIVATE_PCH})
    endif()

    unset(__CURRENT_MODULE_NAME PARENT_SCOPE)
endfunction()
