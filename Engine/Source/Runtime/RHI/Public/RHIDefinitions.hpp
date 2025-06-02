#pragma once
#include "Definitions.hpp"

#define WS_RHI_BACKEND_VULKAN

#ifdef WS_RHI_BACKEND_VULKAN
#include "volk.h"

#define WS_ASSERT_VK(result)                                                   \
    if (result != VK_SUCCESS)                                                  \
    {                                                                          \
        WS_LOG_ERROR("RHI", "Vulkan error: {}", #result);                      \
        WS_ASSERT(result);                                                     \
    }
#endif

namespace worse
{
    enum class RHIBackendType
    {
        Vulkan,
        Null,
    };

    enum class RHIQueueType
    {
        Graphics,
        Compute,
        Transfer,
        Max,
    };

    enum class RHISyncPrimitiveType
    {
        Fence,
        BinarySemaphore,
        TimelineSemaphore,
        Max,
    };

    enum class RHIPresentMode
    {
        Immediate,
        Mailbox,
        FIFO,
        Max
    };

    enum class RHIFormat
    {
        // R
        R8Unorm,
        R8Uint,
        R16Unorm,
        R16Uint,
        R16Float,
        R32Uint,
        R32Float,
        // Rg
        R8G8Unorm,
        R16G16Float,
        R32G32Float,
        // Rgb
        R11G11B10Float,
        R32G32B32Float,
        // Rgba
        R8G8B8A8Unorm,
        R10G10B10A2Unorm,
        R16G16B16A16Unorm,
        R16G16B16A16Snorm,
        R16G16B16A16Float,
        R32G32B32A32Float,
        // Depth
        D16Unorm,
        D32Float,
        D32FloatS8X24Uint,
        // Surface
        B8R8G8A8Unorm,

        Max
    };

    enum class RHIImageLayout
    {
        General,
        Attacement,
        ShaderRead,
        TransferSource,
        TransferDestination,
        PresentSource,
        Max,
    };

    enum class RHIShaderType
    {
        Vertex,
        Fragment,
        Compute,
        Max,
    };

    constexpr VkObjectType vulkanObjectType[] = {
        VK_OBJECT_TYPE_FENCE,
        VK_OBJECT_TYPE_SEMAPHORE,
        VK_OBJECT_TYPE_QUEUE,
        VK_OBJECT_TYPE_BUFFER,
        VK_OBJECT_TYPE_IMAGE,
        VK_OBJECT_TYPE_IMAGE_VIEW,
        VK_OBJECT_TYPE_SAMPLER,
        VK_OBJECT_TYPE_SHADER_MODULE,
        VK_OBJECT_TYPE_PIPELINE,
        VK_OBJECT_TYPE_PIPELINE_LAYOUT,
        VK_OBJECT_TYPE_DESCRIPTOR_SET,
        VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
        VK_OBJECT_TYPE_COMMAND_BUFFER,
        VK_OBJECT_TYPE_COMMAND_POOL,
    };

    constexpr VkFormat vulkanFormat[] = {VK_FORMAT_R8_UNORM,
                                         VK_FORMAT_R8_UINT,
                                         VK_FORMAT_R16_UNORM,
                                         VK_FORMAT_R16_UINT,
                                         VK_FORMAT_R16_SFLOAT,
                                         VK_FORMAT_R32_UINT,
                                         VK_FORMAT_R32_SFLOAT,
                                         VK_FORMAT_R8G8_UNORM,
                                         VK_FORMAT_R16G16_SFLOAT,
                                         VK_FORMAT_R32G32_SFLOAT,
                                         VK_FORMAT_B10G11R11_UFLOAT_PACK32,
                                         VK_FORMAT_R32G32B32_SFLOAT,
                                         VK_FORMAT_R8G8B8A8_UNORM,
                                         VK_FORMAT_A2B10G10R10_UNORM_PACK32,
                                         VK_FORMAT_R16G16B16A16_UNORM,
                                         VK_FORMAT_R16G16B16A16_SNORM,
                                         VK_FORMAT_R16G16B16A16_SFLOAT,
                                         VK_FORMAT_R32G32B32A32_SFLOAT,
                                         VK_FORMAT_D16_UNORM,
                                         VK_FORMAT_D32_SFLOAT,
                                         VK_FORMAT_D32_SFLOAT_S8_UINT,
                                         VK_FORMAT_B8G8R8A8_UNORM};

    struct RHIContext
    {
#ifdef WS_RHI_BACKEND_VULKAN
        static inline VkInstance instance             = VK_NULL_HANDLE;
        static inline VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        static inline VkDevice device                 = VK_NULL_HANDLE;
#endif

        static inline RHIBackendType backendType = RHIBackendType::Null;
    };

} // namespace worse
