set(MODULE_NAME RHI)
add_library(${MODULE_NAME} STATIC)
add_library(Worse::${MODULE_NAME} ALIAS ${MODULE_NAME})

file(GLOB_RECURSE ${MODULE_NAME}_PUBLIC_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Public/*.hpp"
)
file(GLOB_RECURSE ${MODULE_NAME}_PRIVATE_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Private/*.hpp"
)

target_sources(${MODULE_NAME}
    PUBLIC  ${${MODULE_NAME}_PUBLIC_HEADERS}
    PRIVATE ${${MODULE_NAME}_PRIVATE_SOURCES}
)

target_include_directories(${MODULE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
)

target_link_libraries(${MODULE_NAME}
    PRIVATE
        GPUOpen::VulkanMemoryAllocator
        Vulkan-Headers
        DXC
        spirv-cross-hlsl
    PUBLIC
        volk_headers
        Worse::Core
        Worse::FileSystem
        Worse::Asset
)

# Disable nullability completeness warnings for Clang and AppleClang
target_compile_options(${MODULE_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:-Wno-nullability-completeness>
    $<$<CXX_COMPILER_ID:AppleClang>:-Wno-nullability-completeness>
)