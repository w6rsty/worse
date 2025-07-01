#pragma once
#include "Definitions.hpp"
#include "Types.hpp"
#include "RHITypes.hpp"

#define WS_RHI_BACKEND_VULKAN

#ifdef WS_RHI_BACKEND_VULKAN
#include "volk.h"

#ifdef DEBUG
#define WS_ASSERT_VK(result)                                                   \
    do                                                                         \
    {                                                                          \
        VkResult vkResult = (result);                                          \
        if (vkResult != VK_SUCCESS)                                            \
        {                                                                      \
            WS_LOG_ERROR("RHI", "Vulkan error: {}", #result);                  \
            WS_ASSERT(vkResult);                                               \
        }                                                                      \
    } while (false)
#else
#define WS_ASSERT_VK(result)                                                   \
    do                                                                         \
    {                                                                          \
        VkResult vkResult = (result);                                          \
        (void)vkResult;                                                        \
    } while (false)
#endif
#endif

namespace worse
{
    // fwd
    class RHINativeHandle;
    class RHIDevice;
    class RHIQueue;
    class RHIViewport;
    class RHICommandList;
    class RHISwapchain;
    class RHISyncPrimitive;
    class RHIShader;
    class RHIPipeline;
    class RHIPipelineState;
    class RHIRasterizerState;
    class RHIDepthStencilState;
    class RHIBlendState;
    class RHITexture;
    class RHIBuffer;
    class RHISampler;
    class RHIDescriptor;
    class RHIDescriptorSet;
    class RHIDescriptorSetLayout;

    enum class RHIBackendType
    {
        Vulkan,
        Null,
    };

    enum class RHINativeHandleType
    {
        DontCare,
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
        DescriptorPool,
        DescriptorSetLayout,
        DescriptorSet,
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

    enum class RHIImageLayout
    {
        Undefined,
        General,
        ColorAttachment,
        DepthStencilAttachment,
        ShaderRead,
        TransferSource,
        TransferDestination,
        PresentSource,
        Max,
    };

    constexpr VkImageViewType vulkanImageViewType(RHITextureType const type)
    {
        switch (type)
        {
            // clang-format off
        case RHITextureType::Texture2D:      return VK_IMAGE_VIEW_TYPE_2D;
        case RHITextureType::Texture2DArray: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case RHITextureType::Texture3D:      return VK_IMAGE_VIEW_TYPE_3D;
        case RHITextureType::TextureCube:    return VK_IMAGE_VIEW_TYPE_CUBE;
        default:                             return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
            // clang-format on
        }
    }

    constexpr VkFilter vulkanFilter(RHIFilter const filter)
    {
        switch (filter)
        {
            // clang-format off
        case RHIFilter::Nearest: return VK_FILTER_NEAREST;
        case RHIFilter::Linear:  return VK_FILTER_LINEAR;
            // clang-format on
        }
    }

    constexpr VkSamplerMipmapMode
    vulkanSamplerMipmapMode(RHIFilter const filter)
    {
        switch (filter)
        {
            // clang-format off
        case RHIFilter::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case RHIFilter::Linear:  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            // clang-format on
        }
    }

    enum class RHISamplerAddressMode
    {
        Wrap,
        Mirror,
        ClampToEdge,
        ClampToBorder,
        MirrorClampToEdge,
        Max
    };

    constexpr VkSamplerAddressMode
    vulkanSamplerAddressMode(RHISamplerAddressMode const addressMode)
    {
        switch (addressMode)
        {
            // clang-format off
        case RHISamplerAddressMode::Wrap:             return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case RHISamplerAddressMode::Mirror:           return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case RHISamplerAddressMode::ClampToEdge:      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case RHISamplerAddressMode::ClampToBorder:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case RHISamplerAddressMode::MirrorClampToEdge:return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        default:                                      return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
            // clang-format on
        }
    }

    enum class RHISamplerType
    {
        CompareDepth,
        PointClampEdge,
        PointClampBorder,
        Wrap,
        BilinearClampEdge,
        BilinearClampBorder,
        BilinearWrap,
        TrilinearClamp,
        AnisotropicClamp,
        Max
    };

    enum class RHIShaderType
    {
        Vertex,
        Pixel,
        Compute,
        Max,
    };

    enum class RHIPipelineType
    {
        Graphics,
        Compute,
    };

    WS_DEFINE_FLAGS(RHIShaderStage, std::uint32_t);
    // clang-format off
    struct RHIShaderStageFlagBits
    {
        static constexpr RHIShaderStageFlags None   {0x00000000};
        static constexpr RHIShaderStageFlags Vertex {0x00000001};
        static constexpr RHIShaderStageFlags Pixel  {0x00000010};
        static constexpr RHIShaderStageFlags Compute{0x00000020};
        static constexpr RHIShaderStageFlags All    {0x7FFFFFFF};
    };
    constexpr RHIShaderStageFlags RHIComputePipelineShaderCombination  = RHIShaderStageFlagBits::Compute;
    constexpr RHIShaderStageFlags RHIGraphicsPipelineShaderCombination = RHIShaderStageFlagBits::Vertex | RHIShaderStageFlagBits::Pixel;
    // clang-format on

    // conver RHI shader type to RHI shader stage flags
    constexpr RHIShaderStageFlags rhiShaderStageFlags(RHIShaderType const type)
    {
        switch (type)
        {
            // clang-format off
        case RHIShaderType::Vertex:   return RHIShaderStageFlagBits::Vertex;
        case RHIShaderType::Pixel:    return RHIShaderStageFlagBits::Pixel;
        case RHIShaderType::Compute:  return RHIShaderStageFlagBits::Compute;
        default:                      return RHIShaderStageFlagBits::All;
            // clang-format on
        }
    }

    // convert RHI shader type to singoe vulkan shader stage flags
    constexpr VkShaderStageFlags
    vulkanShaderStageFlags(RHIShaderType const type)
    {
        switch (type)
        {
            // clang-format off
        case RHIShaderType::Vertex:   return VK_SHADER_STAGE_VERTEX_BIT;
        case RHIShaderType::Pixel:    return VK_SHADER_STAGE_FRAGMENT_BIT;
        case RHIShaderType::Compute:  return VK_SHADER_STAGE_COMPUTE_BIT;
        default:                      return VK_SHADER_STAGE_ALL;
            // clang-format on
        }
    }

    // convert RHI shader stage flags to vulkan shader stage flags
    constexpr VkShaderStageFlags
    vulkanShaderStageFlags(RHIShaderStageFlags const stageFlags)
    {
        VkShaderStageFlags flags = 0;
        if (stageFlags & RHIShaderStageFlagBits::Vertex)
        {
            flags |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (stageFlags & RHIShaderStageFlagBits::Pixel)
        {
            flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if (stageFlags & RHIShaderStageFlagBits::Compute)
        {
            flags |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
        return flags;
    }

    enum class RHIPolygonMode
    {
        Solid,
        Wirefame,
    };

    enum class RHICullMode
    {
        None,
        Front,
        Back,
    };

    enum class RHIFrontFace
    {
        CW,
        CCW,
    };

    enum class RHIPrimitiveTopology
    {
        PointList,
        LineList,
        TriangleList,
    };

    constexpr std::string
    rhiPrimitiveTopologyToString(RHIPrimitiveTopology const topology)
    {
        switch (topology)
        {
            // clang-format off
        case RHIPrimitiveTopology::PointList:     return "PointList";
        case RHIPrimitiveTopology::LineList:      return "LineList";
        case RHIPrimitiveTopology::TriangleList:  return "TriangleList";
        default:                                   return "Unknown";
            // clang-format on
        }
    }

    enum class RHICompareOperation
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    enum class RHIStencilOperation
    {
        Keep,
        Zero,
        Replace,
        IncrementAndClamp,
        DecrementAndClamp,
        Invert,
        IncrementAndWrap,
        DecrementAndWrap
    };

    enum class RHIBlendFactor
    {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstColor,
        OneMinusDstColor,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        SrcAlphaSaturate,
    };

    enum class RHIBlendOperation
    {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max,
    };

    enum class RHIDescriptorType
    {
        Texture,
        TextureStorage,
        PushConstant,
        UniformBuffer,
        StructuredBuffer,
        RWStructuredBuffer,
        Max
    };

    constexpr VkDescriptorType
    vulkanDescriptorType(RHIDescriptorType const type)
    {
        switch (type)
        {
            // clang-format off
        case RHIDescriptorType::Texture:            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case RHIDescriptorType::TextureStorage:     return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case RHIDescriptorType::UniformBuffer:      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case RHIDescriptorType::StructuredBuffer:
        case RHIDescriptorType::RWStructuredBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        default:                                    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
            // clang-format on
        }
    }

    constexpr std::string
    rhiDescriptorTypeToString(RHIDescriptorType const type)
    {
        switch (type)
        {
            // clang-format off
        case RHIDescriptorType::Texture:            return "Texture";
        case RHIDescriptorType::TextureStorage:     return "RWTexture";
        case RHIDescriptorType::PushConstant:       return "PushConstant";
        case RHIDescriptorType::UniformBuffer:      return "UniformBuffer";
        case RHIDescriptorType::StructuredBuffer:   return "StructuredBuffer";
        case RHIDescriptorType::RWStructuredBuffer: return "RWStructuredBuffer";
        default:                                    return "Unknown";
            // clang-format on
        }
    }

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
        case RHINativeHandleType::DescriptorPool:      return VK_OBJECT_TYPE_DESCRIPTOR_POOL;
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

    constexpr VkFormat vulkanFormat(RHIFormat const format)
    {
        switch (format)
        {
            // clang-format off
        case RHIFormat::R8Unorm:           return VK_FORMAT_R8_UNORM;
        case RHIFormat::R8Uint:            return VK_FORMAT_R8_UINT;
        case RHIFormat::R16Unorm:          return VK_FORMAT_R16_UNORM;
        case RHIFormat::R16Uint:           return VK_FORMAT_R16_UINT;
        case RHIFormat::R16Float:          return VK_FORMAT_R16_SFLOAT;
        case RHIFormat::R32Uint:           return VK_FORMAT_R32_UINT;
        case RHIFormat::R32Float:          return VK_FORMAT_R32_SFLOAT;
        case RHIFormat::R8G8Unorm:         return VK_FORMAT_R8G8_UNORM;
        case RHIFormat::R16G16Float:       return VK_FORMAT_R16G16_SFLOAT;
        case RHIFormat::R32G32Float:       return VK_FORMAT_R32G32_SFLOAT;
        case RHIFormat::R11G11B10Float:    return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case RHIFormat::R32G32B32Float:    return VK_FORMAT_R32G32B32_SFLOAT;
        case RHIFormat::R8G8B8A8Unorm:     return VK_FORMAT_R8G8B8A8_UNORM;
        case RHIFormat::B8R8G8A8Unorm:     return VK_FORMAT_B8G8R8A8_UNORM;
        case RHIFormat::R10G10B10A2Unorm:  return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case RHIFormat::R16G16B16A16Unorm: return VK_FORMAT_R16G16B16A16_UNORM;
        case RHIFormat::R16G16B16A16Snorm: return VK_FORMAT_R16G16B16A16_SNORM;
        case RHIFormat::R16G16B16A16Float: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case RHIFormat::R32G32B32A32Float: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case RHIFormat::D16Unorm:          return VK_FORMAT_D16_UNORM;
        case RHIFormat::D32Float:          return VK_FORMAT_D32_SFLOAT;
        case RHIFormat::D32FloatS8X24Uint: return VK_FORMAT_D32_SFLOAT_S8_UINT;
        default:                           return VK_FORMAT_UNDEFINED;
            // clang-format on
        }
    }

    constexpr VkImageType vulkanImageType(RHITextureType const type)
    {
        switch (type)
        {
            // clang-format off
        case RHITextureType::Texture2D:      return VK_IMAGE_TYPE_2D;
        case RHITextureType::Texture2DArray: return VK_IMAGE_TYPE_2D;
        case RHITextureType::Texture3D:      return VK_IMAGE_TYPE_3D;
        case RHITextureType::TextureCube:    return VK_IMAGE_TYPE_2D;
        default:                             return VK_IMAGE_TYPE_MAX_ENUM;
            // clang-format on
        }
    }

    // Notice that RHIImageLayout::Max -> VK_IMAGE_LAYOUT_UNDEFINED
    constexpr VkImageLayout vulkanImageLayout(RHIImageLayout const layout)
    {
        switch (layout)
        {
            // clang-format off
        case RHIImageLayout::Undefined:              return VK_IMAGE_LAYOUT_UNDEFINED;
        case RHIImageLayout::General:                return VK_IMAGE_LAYOUT_GENERAL;
        case RHIImageLayout::ColorAttachment:        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case RHIImageLayout::DepthStencilAttachment: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case RHIImageLayout::ShaderRead:             return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case RHIImageLayout::TransferSource:         return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case RHIImageLayout::TransferDestination:    return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case RHIImageLayout::PresentSource:          return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case RHIImageLayout::Max:                    return VK_IMAGE_LAYOUT_UNDEFINED;
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

    constexpr VkPolygonMode vulkanPolygonMode(RHIPolygonMode const mode)
    {
        switch (mode)
        {
            // clang-format off
        case RHIPolygonMode::Solid:      return VK_POLYGON_MODE_FILL;
        case RHIPolygonMode::Wirefame:  return VK_POLYGON_MODE_LINE;
        default:                        return VK_POLYGON_MODE_FILL;
            // clang-format on
        }
    }
    constexpr VkCullModeFlags vulkanCullModeFlags(RHICullMode const mode)
    {
        switch (mode)
        {
            // clang-format off
        case RHICullMode::None:  return VK_CULL_MODE_NONE;
        case RHICullMode::Front: return VK_CULL_MODE_FRONT_BIT;
        case RHICullMode::Back:  return VK_CULL_MODE_BACK_BIT;
        default:                 return VK_CULL_MODE_NONE;
            // clang-format on
        }
    }

    constexpr VkFrontFace vulkanFrontFace(RHIFrontFace const frontFace)
    {
        switch (frontFace)
        {
            // clang-format off
        case RHIFrontFace::CW:  return VK_FRONT_FACE_CLOCKWISE;
        case RHIFrontFace::CCW: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        default:                return VK_FRONT_FACE_CLOCKWISE;
            // clang-format on
        }
    }

    constexpr VkPrimitiveTopology
    vulkanPrimitiveTopology(RHIPrimitiveTopology const topology)
    {
        switch (topology)
        {
            // clang-format off
        case RHIPrimitiveTopology::PointList:   return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case RHIPrimitiveTopology::LineList:    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case RHIPrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default:                                return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
            // clang-format on
        }
    }

    constexpr VkCompareOp vulkanCompareOp(RHICompareOperation const op)
    {
        switch (op)
        {
            // clang-format off
        case RHICompareOperation::Never:        return VK_COMPARE_OP_NEVER;
        case RHICompareOperation::Less:         return VK_COMPARE_OP_LESS;
        case RHICompareOperation::Equal:        return VK_COMPARE_OP_EQUAL;
        case RHICompareOperation::LessEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case RHICompareOperation::Greater:      return VK_COMPARE_OP_GREATER;
        case RHICompareOperation::NotEqual:     return VK_COMPARE_OP_NOT_EQUAL;
        case RHICompareOperation::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case RHICompareOperation::Always:       return VK_COMPARE_OP_ALWAYS;
        default:                                return VK_COMPARE_OP_MAX_ENUM;
            // clang-format on
        }
    }

    constexpr VkStencilOp vulkanStencilOp(RHIStencilOperation const op)
    {
        switch (op)
        {
            // clang-format off
        case RHIStencilOperation::Keep:              return VK_STENCIL_OP_KEEP;
        case RHIStencilOperation::Zero:              return VK_STENCIL_OP_ZERO;
        case RHIStencilOperation::Replace:           return VK_STENCIL_OP_REPLACE;
        case RHIStencilOperation::IncrementAndClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case RHIStencilOperation::DecrementAndClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case RHIStencilOperation::Invert:            return VK_STENCIL_OP_INVERT;
        case RHIStencilOperation::IncrementAndWrap:  return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case RHIStencilOperation::DecrementAndWrap:  return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        default:                                     return VK_STENCIL_OP_MAX_ENUM;
            // clang-format on
        }
    }

    constexpr VkBlendFactor vulkanBlendFactor(RHIBlendFactor const factor)
    {
        switch (factor)
        {
            // clang-format off
        case RHIBlendFactor::Zero:                  return VK_BLEND_FACTOR_ZERO;
        case RHIBlendFactor::One:                   return VK_BLEND_FACTOR_ONE;
        case RHIBlendFactor::SrcColor:              return VK_BLEND_FACTOR_SRC_COLOR;
        case RHIBlendFactor::OneMinusSrcColor:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case RHIBlendFactor::SrcAlpha:              return VK_BLEND_FACTOR_SRC_ALPHA;
        case RHIBlendFactor::OneMinusSrcAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case RHIBlendFactor::DstColor:              return VK_BLEND_FACTOR_DST_COLOR;
        case RHIBlendFactor::OneMinusDstColor:      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case RHIBlendFactor::DstAlpha:              return VK_BLEND_FACTOR_DST_ALPHA;
        case RHIBlendFactor::OneMinusDstAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case RHIBlendFactor::ConstantColor:         return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case RHIBlendFactor::OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case RHIBlendFactor::ConstantAlpha:         return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case RHIBlendFactor::OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case RHIBlendFactor::SrcAlphaSaturate:      return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        default:                                    return VK_BLEND_FACTOR_MAX_ENUM;
            // clang-format on
        }
    }

    constexpr VkBlendOp vulkanBlendOp(RHIBlendOperation const op)
    {
        switch (op)
        {
            // clang-format off
        case RHIBlendOperation::Add:             return VK_BLEND_OP_ADD;
        case RHIBlendOperation::Subtract:        return VK_BLEND_OP_SUBTRACT;
        case RHIBlendOperation::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case RHIBlendOperation::Min:             return VK_BLEND_OP_MIN;
        case RHIBlendOperation::Max:             return VK_BLEND_OP_MAX;
        default:                                 return VK_BLEND_OP_MAX_ENUM;
            // clang-format on
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
            // clang-format on
        }
    }

    struct RHIContext
    {
#ifdef WS_RHI_BACKEND_VULKAN

        static inline std::uint32_t version           = VK_API_VERSION_1_3;
        static inline VkInstance instance             = VK_NULL_HANDLE;
        static inline VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        static inline VkDevice device                 = VK_NULL_HANDLE;
#endif

        static inline RHIBackendType backendType = RHIBackendType::Null;
    };

    namespace RHIConfig
    {
        static bool enableVSync            = true;
        static bool enableValidationLayers = true;

        constexpr std::size_t MAX_RENDER_TARGET = 8;
        // Minimum descriptor for initial descriptor pool
        constexpr std::uint32_t MIN_DESCRIPTORS             = 512;
        constexpr std::uint32_t MAX_DESCRIPTORS             = 2048;
        constexpr std::uint32_t MAX_DESCRIPTOR_SETS         = 512;
        constexpr std::uint32_t MAX_DESCRIPTOR_SET_BINDINGS = 256;
        constexpr std::size_t MAX_BUFFER_UPDATE_SIZE = 64 * 1024; // 64 KB
        constexpr std::size_t MAX_PUSH_CONSTANT_SIZE = 128;       // 128 bytes

        constexpr std::uint32_t HLSL_REGISTER_SHIFT_B = 0;
        constexpr std::uint32_t HLSL_REGISTER_SHIFT_S = 100;
        constexpr std::uint32_t HLSL_REGISTER_SHIFT_U = 200;
        constexpr std::uint32_t HLSL_REGISTER_SHIFT_T = 300;
    } // namespace RHIConfig

} // namespace worse
