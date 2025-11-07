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

    message(STATUS "")
    message(STATUS "Collected thirdparty targets:")
    foreach(third_party_target ${THIRD_PARTY_TARGETS})
        message(STATUS "- ${third_party_target}")
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

    foreach(engine_module ${ENGINE_MODULE_TARGETS})
        DisplayModuleProperties(${engine_module})
    endforeach()

    set(${OUT_VAR} "${ENGINE_MODULE_TARGETS}" PARENT_SCOPE)
endfunction()

# Resolve engine module dependencies and definitions
# ARGN: List of declared internal and external dependencies
function(ResolveEngineModules)
    set(DECLARED_DEPENDENCIES ${ARGN})
    
    function(__ResolveModule MODULE_NAME OUT_VAR)
        get_target_property(public_deps ${MODULE_NAME} PUBLIC_DEPENDENCIES)
        get_target_property(private_deps ${MODULE_NAME} PRIVATE_DEPENDENCIES)

        set(MISSING_DEPENDENCIES "")
        foreach(dep ${public_deps})
            if (NOT dep IN_LIST DECLARED_DEPENDENCIES)
                list(APPEND MISSING_DEPENDENCIES ${dep})
            else()
                target_link_libraries(${MODULE_NAME} PUBLIC ${dep})
            endif()
        endforeach()
        foreach(dep ${private_deps})
            if (NOT dep IN_LIST DECLARED_DEPENDENCIES)
                list(APPEND MISSING_DEPENDENCIES ${dep})
            else()
                target_link_libraries(${MODULE_NAME} PRIVATE ${dep})
            endif()
        endforeach()

        set(${OUT_VAR} "${MISSING_DEPENDENCIES}" PARENT_SCOPE)
    endfunction()

    set(MISSING_DEPS_COUNT 0)
    foreach(engine_module ${ENGINE_MODULE_TARGETS})
        __ResolveModule(${engine_module} MISSING_DEPS)

        if(MISSING_DEPS)
            list(LENGTH MISSING_DEPS NUM_MISSING)
            math(EXPR MISSING_DEPS_COUNT "${MISSING_DEPS_COUNT} + ${NUM_MISSING}")

            message(WARNING "'${engine_module}' has unresolved dependencies: ${MISSING_DEPS}")
        endif()
    endforeach()
    if (MISSING_DEPS_COUNT GREATER 0)
        message(FATAL_ERROR "Incomplete module dependencies.")
    endif()

endfunction()