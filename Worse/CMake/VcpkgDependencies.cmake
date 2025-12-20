include_guard()

function(CollectVcpkgDependencies OUT_VAR)
    set(VCPKG_DEPENDENCIES_TARGETS "")

    find_package(SDL3 CONFIG REQUIRED)
    list(APPEND VCPKG_DEPENDENCIES_TARGETS "SDL3::SDL3")

    find_package(slang CONFIG REQUIRED)
    list(APPEND VCPKG_DEPENDENCIES_TARGETS "slang::gfx" "slang::slang" "slang::slang-llvm" "slang::slang-glslang")

    find_package(spdlog CONFIG REQUIRED)
    list(APPEND VCPKG_DEPENDENCIES_TARGETS "spdlog::spdlog" "spdlog::spdlog_header_only")

    find_package(VulkanHeaders CONFIG REQUIRED)
    list(APPEND VCPKG_DEPENDENCIES_TARGETS "Vulkan::Headers")

    find_package(VulkanMemoryAllocator CONFIG REQUIRED)
    list(APPEND VCPKG_DEPENDENCIES_TARGETS "VulkanMemoryAllocator::VulkanMemoryAllocator")

    message(STATUS "")
    message(STATUS "Collected vcpkg targets:")
    foreach(vcpkg_target ${VCPKG_DEPENDENCIES_TARGETS})
        message(STATUS "- ${vcpkg_target}")
    endforeach()

    set(${OUT_VAR} "${VCPKG_DEPENDENCIES_TARGETS}" PARENT_SCOPE)
endfunction()
