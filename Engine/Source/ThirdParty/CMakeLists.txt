add_subdirectory(dxc)

add_subdirectory(imgui)
set_target_properties(imgui PROPERTIES FOLDER "ThirdParty")

add_subdirectory(SDL3)

set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "" FORCE)
add_subdirectory(SPIRV-Cross)
set_target_properties(
    spirv-cross-c
    spirv-cross-core
    spirv-cross-cpp
    spirv-cross-glsl
    spirv-cross-hlsl
    spirv-cross-reflect
    spirv-cross-util
    PROPERTIES FOLDER "ThirdParty"
)

set(STB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE
    ${STB_ROOT}
)

add_subdirectory(volk)
set_target_properties(volk PROPERTIES FOLDER "ThirdParty")

add_subdirectory(Vulkan-Headers)
add_subdirectory(VulkanMemoryAllocator)

