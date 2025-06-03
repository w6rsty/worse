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

    enum class RHINativeHandleType
    {
        Fence,
        Semaphore,
        Queue,
        Buffer,
        Image,
        ImageView,
        Sampler,
        Shader,
        Pipeline,
        PipelineLayout,
        DescriptorSet,
        DescriptorSetLayout,
        CommandList,
        CommandPool,

        Swapchain,
        Surface,
        Max,
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
        B8R8G8A8Unorm,
        R10G10B10A2Unorm,
        R16G16B16A16Unorm,
        R16G16B16A16Snorm,
        R16G16B16A16Float,
        R32G32B32A32Float,
        // Depth
        D16Unorm,
        D32Float,
        D32FloatS8X24Uint,

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

    constexpr VkObjectType vulkanObjectType(RHINativeHandleType const type)
    {
        switch (type)
        {
            // clang-format off
        case RHINativeHandleType::Fence:               return VK_OBJECT_TYPE_FENCE;
        case RHINativeHandleType::Semaphore:           return VK_OBJECT_TYPE_SEMAPHORE;
        case RHINativeHandleType::Queue:               return VK_OBJECT_TYPE_QUEUE;
        case RHINativeHandleType::Buffer:              return VK_OBJECT_TYPE_BUFFER;
        case RHINativeHandleType::Image:               return VK_OBJECT_TYPE_IMAGE;
        case RHINativeHandleType::ImageView:           return VK_OBJECT_TYPE_IMAGE_VIEW;
        case RHINativeHandleType::Sampler:             return VK_OBJECT_TYPE_SAMPLER;
        case RHINativeHandleType::Shader:              return VK_OBJECT_TYPE_SHADER_MODULE;
        case RHINativeHandleType::Pipeline:            return VK_OBJECT_TYPE_PIPELINE;
        case RHINativeHandleType::PipelineLayout:      return VK_OBJECT_TYPE_PIPELINE_LAYOUT;
        case RHINativeHandleType::DescriptorSet:       return VK_OBJECT_TYPE_DESCRIPTOR_SET;
        case RHINativeHandleType::DescriptorSetLayout: return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
        case RHINativeHandleType::CommandList:         return VK_OBJECT_TYPE_COMMAND_BUFFER;
        case RHINativeHandleType::CommandPool:         return VK_OBJECT_TYPE_COMMAND_POOL;
        case RHINativeHandleType::Swapchain:           return VK_OBJECT_TYPE_SWAPCHAIN_KHR;
        case RHINativeHandleType::Surface:             return VK_OBJECT_TYPE_SURFACE_KHR;
        default:                                       return VK_OBJECT_TYPE_UNKNOWN;
            // clang-format on
        }
    }

    constexpr VkImageAspectFlags vulkanImageAspectFlags(RHIFormat const format)
    {
        switch (format)
        {
        case RHIFormat::D16Unorm:
        case RHIFormat::D32Float:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case RHIFormat::D32FloatS8X24Uint:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    constexpr std::string rhiFormatToString(RHIFormat const format)
    {
        switch (format)
        {
            // clang-format off
#define WS_RHI_FORMAT_CASE(name) case RHIFormat::name: return #name
        WS_RHI_FORMAT_CASE(R8Unorm);
        WS_RHI_FORMAT_CASE(R8Uint);
        WS_RHI_FORMAT_CASE(R16Unorm);
        WS_RHI_FORMAT_CASE(R16Uint);
        WS_RHI_FORMAT_CASE(R16Float);
        WS_RHI_FORMAT_CASE(R32Uint);
        WS_RHI_FORMAT_CASE(R32Float);
        WS_RHI_FORMAT_CASE(R8G8Unorm);
        WS_RHI_FORMAT_CASE(R16G16Float);
        WS_RHI_FORMAT_CASE(R32G32Float);
        WS_RHI_FORMAT_CASE(R11G11B10Float);
        WS_RHI_FORMAT_CASE(R32G32B32Float);
        WS_RHI_FORMAT_CASE(R8G8B8A8Unorm);
        WS_RHI_FORMAT_CASE(R10G10B10A2Unorm);
        WS_RHI_FORMAT_CASE(R16G16B16A16Unorm);
        WS_RHI_FORMAT_CASE(R16G16B16A16Snorm);
        WS_RHI_FORMAT_CASE(R16G16B16A16Float);
        WS_RHI_FORMAT_CASE(R32G32B32A32Float);
        WS_RHI_FORMAT_CASE(D16Unorm);
        WS_RHI_FORMAT_CASE(D32Float);
        WS_RHI_FORMAT_CASE(D32FloatS8X24Uint);
        WS_RHI_FORMAT_CASE(B8R8G8A8Unorm);
        WS_RHI_FORMAT_CASE(Max);
#undef WS_RHI_FORMAT_CASE
            break;
        }
    }

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
