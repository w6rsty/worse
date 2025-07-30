add_subdirectory(dxc)
add_subdirectory(imgui)
add_subdirectory(SDL3)

set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(SPIRV-Cross)

set(STB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE
    ${STB_ROOT}
)

add_subdirectory(volk)
add_subdirectory(Vulkan-Headers)
add_subdirectory(VulkanMemoryAllocator)