include_guard()

# Recursively collects absolute paths of CMake subdirectories under ROOT_DIR
# ROOT_DIR:
# OUT_VAR: List of all subdirectories
function(CollectAllSourceDirectories ROOT_DIR OUT_VAR)
    set(ALL_DIRS "")

    file(REAL_PATH ${ROOT_DIR} ROOT_ABS)
    file(GLOB CURRENT_SUBDIRS LIST_DIRECTORIES TRUE RELATIVE "${ROOT_ABS}" "${ROOT_ABS}/*")

    foreach(subdir ${CURRENT_SUBDIRS})
        set(SUBDIR_ABS "${ROOT_ABS}/${subdir}")

        if(IS_DIRECTORY "${SUBDIR_ABS}")
            CollectAllSourceDirectories("${SUBDIR_ABS}" CHILD_DIRS)
            list(APPEND ALL_DIRS ${CHILD_DIRS})

            if(EXISTS "${SUBDIR_ABS}/CMakeLists.txt")
                list(APPEND ALL_DIRS "${SUBDIR_ABS}")
            endif()
        endif()
    endforeach()

    set(${OUT_VAR} "${ALL_DIRS}" PARENT_SCOPE)
endfunction()

# Collect subdirectories under ROOT_DIR (only one level)
# ROOT_DIR:
# OUT_VAR: List of level one subdirectories
function(CollectSubSourceDirectories ROOT_DIR OUT_VAR)
    set(ALL_DIRS "")

    file(REAL_PATH ${ROOT_DIR} ROOT_ABS)
    file(GLOB CURRENT_SUBDIRS LIST_DIRECTORIES TRUE RELATIVE "${ROOT_ABS}" "${ROOT_ABS}/*")

    foreach(subdir ${CURRENT_SUBDIRS})
        set(SUBDIR_ABS "${ROOT_ABS}/${subdir}")

        if(IS_DIRECTORY "${SUBDIR_ABS}")
            if(EXISTS "${SUBDIR_ABS}/CMakeLists.txt")
                list(APPEND ALL_DIRS "${SUBDIR_ABS}")
            endif()
        endif()
    endforeach()

    set(${OUT_VAR} "${ALL_DIRS}" PARENT_SCOPE)
endfunction()


# Add targets into folder which shows in IDEs
# FOLDER_NAME: The folder name to assign
# ARGN: The list of target names to classify
function(ClassifyTargets FOLDER_NAME)
    foreach (target_name ${ARGN})
        set_target_properties(${target_name} PROPERTIES FOLDER ${FOLDER_NAME})
    endforeach()
endfunction()