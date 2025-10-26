include_guard()
include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/Common.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/Platform.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/Module.cmake)

# Collect all third party targets
# OUT_VAR: List of third party targets 
function(CollectThirdPartyTargets OUT_VAR)
    set(THIRD_PARTY_ROOT_DIR ${WORSE_ROOT_DIR}/Source/ThirdParty)
    set(THIRD_PARTY_TARGETS "")

    CollectSubSourceDirectories(${THIRD_PARTY_ROOT_DIR} THIRD_PARTY_DIRS)
    foreach(third_party_dir ${THIRD_PARTY_DIRS})
        add_subdirectory(${third_party_dir})
        
        get_property(target_names DIRECTORY ${third_party_dir} PROPERTY BUILDSYSTEM_TARGETS)
        list(APPEND THIRD_PARTY_TARGETS ${target_names})

    file(RELATIVE_PATH relative_dir ${THIRD_PARTY_ROOT_DIR} ${third_party_dir})

    ClassifyTargets("Engine/Source/ThirdParty/${relative_dir}" ${target_names})
    endforeach()

    set(${OUT_VAR} "${THIRD_PARTY_TARGETS}" PARENT_SCOPE)
endfunction()

# Collect all engine module targets
# OUT_VAR: List of engine module targets
function(CollectEngineModules OUT_VAR)
    set(ENGINE_MODULE_TARGETS "")
    
    set(ENGINE_ROOT_DIR ${WORSE_ROOT_DIR}/Source/Engine)
    CollectAllSourceDirectories(${ENGINE_ROOT_DIR} ENGINE_MODULE_DIRS)
    foreach(module_dir ${ENGINE_MODULE_DIRS})
        add_subdirectory(${module_dir})

        # Each engine module has only one target
        get_property(module_name DIRECTORY ${module_dir} PROPERTY BUILDSYSTEM_TARGETS)
        list(APPEND ENGINE_MODULE_TARGETS ${module_name})

        ClassifyTargets("Engine/Source/Runtime" ${module_name})
    endforeach()

    set(${OUT_VAR} "${ENGINE_MODULE_TARGETS}" PARENT_SCOPE)
endfunction()

# Resolve engine module dependencies and definitions
# ARGN: List of declared internal and external dependencies
function(ResolveEngineModules)
    set(DECLARED_DEPENDENCIES ${ARGN})
endfunction()

# Output formatted properties
# message(STATUS "Declared engine module: [${__CURRENT_MODULE_NAME}]")
# if (__${__CURRENT_MODULE_NAME}_PUBLIC_DEPENDENCIES)
#     message(STATUS "  Public Dependencies: ${__${__CURRENT_MODULE_NAME}_PUBLIC_DEPENDENCIES}")
# endif()
# if (__${__CURRENT_MODULE_NAME}_PRIVATE_DEPENDENCIES)
#     message(STATUS "  Private Dependencies: ${__${__CURRENT_MODULE_NAME}_PRIVATE_DEPENDENCIES}")
# endif()
# if (__${__CURRENT_MODULE_NAME}_PUBLIC_DEFINITIONS)
#     message(STATUS "  Public Definitions: ${__${__CURRENT_MODULE_NAME}_PUBLIC_DEFINITIONS}")
# endif()
# if (__${__CURRENT_MODULE_NAME}_PRIVATE_DEFINITIONS)
#     message(STATUS "  Private Definitions: ${__${__CURRENT_MODULE_NAME}_PRIVATE_DEFINITIONS}")
# endif()
# if (__${__CURRENT_MODULE_NAME}_CPP_STANDARD)
#     message(STATUS "  C++ Standard: ${__${__CURRENT_MODULE_NAME}_CPP_STANDARD}")
# endif()