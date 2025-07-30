add_subdirectory(dxc)

add_subdirectory(imgui)
set_target_properties(imgui PROPERTIES FOLDER "ThirdParty")

add_subdirectory(SDL3)

set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(SPIRV-Reflect)
set_target_properties(
    spirv-reflect-static
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

