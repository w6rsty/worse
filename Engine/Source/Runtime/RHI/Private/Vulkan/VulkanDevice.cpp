#include "DXCompiler.hpp" // Do not move
#include "Log.hpp"
#include "Math/Hash.hpp"
#include "RHIQueue.hpp"
#include "RHIDevice.hpp"
#include "RHIShader.hpp"
#include "RHICommandList.hpp"
#include "Pipeline/RHIPipeline.hpp"
#include "Pipeline/RHIPipelineState.hpp"
#include "Descriptor/RHIBuffer.hpp"
#include "Descriptor/RHITexture.hpp"
#include "Descriptor/RHISampler.hpp"
#include "Descriptor/RHIDescriptor.hpp"
#include "Descriptor/RHIDescriptorSet.hpp"
#include "Descriptor/RHIDescriptorSetLayout.hpp"

#include "SDL3/SDL_vulkan.h"
#include "vk_mem_alloc.h"

#include <span>
#include <mutex>
#include <array>
#include <vector>
#include <limits>
#include <memory>
#include <cstdint>
#include <semaphore> // synchronize immediate command
#include <unordered_map>

namespace worse
{
    namespace config
    {
        // require vulkan version
        std::uint32_t version = VK_API_VERSION_1_3;
    } // namespace config

    namespace validation
    {
        char const* name{"VK_LAYER_KHRONOS_validation"}; // macOS
    } // namespace validation

    namespace extensions
    {
        std::vector extensionsInstance{
            VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
        };

        std::vector extensionsDevice{
            "VK_KHR_portability_subset",     // macOS
            VK_KHR_SWAPCHAIN_EXTENSION_NAME, // presentation
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};

        std::vector<char const*> getExtensionsInstance()
        {
            std::uint32_t sdlExtensionCount{0};
            auto sdlExtensions =
                SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
            for (std::uint32_t i = 0; i < sdlExtensionCount; ++i)
            {
                extensionsInstance.emplace_back(sdlExtensions[i]);
            }

            if (RHIConfig::enableValidationLayers)
            {
                extensionsInstance.emplace_back("VK_EXT_debug_utils");
            }
            return extensionsInstance;
        }

        std::vector<char const*> getExtensionsDevice()
        {
            return extensionsDevice;
        }
    } // namespace extensions

    namespace debugMessenger
    {
        VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;

        /// Vulkan debug messenger callback
        static VKAPI_ATTR VkBool32 VKAPI_CALL
        messageFn(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                  VkDebugUtilsMessageTypeFlagsEXT type,
                  VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                  void* pUserData)
        {
            (void)type;

            switch (severity)
            {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                return VK_FALSE;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                WS_LOG_WARN("Vulkan", "{}", pCallbackData->pMessage);
                return VK_FALSE;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                WS_LOG_ERROR("Vulkan", "{}", pCallbackData->pMessage);
                WS_ASSERT(false);
                return VK_FALSE;
            default:
                return VK_FALSE;
            }
        }

        VkDebugUtilsMessengerCreateInfoEXT info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugMessenger::messageFn,
        };

        void create()
        {
            WS_ASSERT_VK(
                vkCreateDebugUtilsMessengerEXT(RHIContext::instance,
                                               &debugMessenger::info,
                                               nullptr,
                                               &debugMessenger::messenger));
        }

        void destory()
        {
            vkDestroyDebugUtilsMessengerEXT(RHIContext::instance,
                                            debugMessenger::messenger,
                                            nullptr);
        }

    } // namespace debugMessenger

    namespace physicalDevice
    {
        void select()
        {
            std::uint32_t gpuCount{0};
            WS_ASSERT_VK(vkEnumeratePhysicalDevices(RHIContext::instance,
                                                    &gpuCount,
                                                    nullptr));
            std::vector<VkPhysicalDevice> gpus(gpuCount);
            WS_ASSERT_VK(vkEnumeratePhysicalDevices(RHIContext::instance,
                                                    &gpuCount,
                                                    gpus.data()));
            WS_ASSERT_MSG(!gpus.empty(),
                          "No physical devices found for Vulkan");

            RHIContext::physicalDevice = gpus[0]; // APPLE M3
        }
    } // namespace physicalDevice

    namespace deviceFeatures
    {
        void* featureChain = nullptr;
        // clang-format off
        VkPhysicalDeviceTimelineSemaphoreFeatures featureTimelineSemaphore   = {};
        VkPhysicalDeviceDynamicRenderingFeatures featureDynamicRendering     = {};
        VkPhysicalDeviceSynchronization2Features featureSynchronization2     = {};
        VkPhysicalDeviceDescriptorIndexingFeatures featureDescriptorIndexing = {};
        // clang-format on

        void detect()
        {
            // clang-format off
            featureDescriptorIndexing.sType                                         = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
            featureDescriptorIndexing.runtimeDescriptorArray                        = VK_TRUE;
            featureDescriptorIndexing.descriptorBindingVariableDescriptorCount      = VK_TRUE;
            featureDescriptorIndexing.descriptorBindingPartiallyBound               = VK_TRUE;
            featureDescriptorIndexing.shaderSampledImageArrayNonUniformIndexing     = VK_TRUE;
            featureDescriptorIndexing.descriptorBindingSampledImageUpdateAfterBind  = VK_TRUE;
            featureDescriptorIndexing.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
            featureDescriptorIndexing.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
            featureDescriptorIndexing.pNext                                         = nullptr;

            featureSynchronization2.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
            featureSynchronization2.synchronization2 = VK_TRUE;
            featureSynchronization2.pNext            = &featureDescriptorIndexing;

            featureDynamicRendering.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
            featureDynamicRendering.dynamicRendering = VK_TRUE;
            featureDynamicRendering.pNext            = &featureSynchronization2;

            featureTimelineSemaphore.sType             = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
            featureTimelineSemaphore.timelineSemaphore = VK_TRUE;
            featureTimelineSemaphore.pNext             = &featureDynamicRendering;
            // clang-format on

            featureChain = &featureTimelineSemaphore;
        }
    } // namespace deviceFeatures

    namespace queues
    {
        RHINativeHandle graphics = {};
        RHINativeHandle compute  = {};
        RHINativeHandle transfer = {};

        std::uint32_t indexGraphics = std::numeric_limits<std::uint32_t>::max();
        std::uint32_t indexCompute  = std::numeric_limits<std::uint32_t>::max();
        std::uint32_t indexTransfer = std::numeric_limits<std::uint32_t>::max();

        std::array<std::shared_ptr<RHIQueue>, k_rhiQueueTypeCount> regular;

        std::mutex mtxImmediateCmd;
        bool isImmediateCmdActive = false;
        std::binary_semaphore semImmediateCmd{1};
        RHIQueue* activeQueue = nullptr;
        std::array<std::shared_ptr<RHIQueue>, k_rhiQueueTypeCount> immediate;

        std::uint32_t getQueueFamilyIndex(
            std::vector<VkQueueFamilyProperties> const& queueFamilies,
            VkQueueFlags flags, bool dedicated = true)
        {
            // compute only
            if ((flags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
            {
                for (std::uint32_t i = 0;
                     i < static_cast<std::uint32_t>(queueFamilies.size());
                     ++i)
                {
                    if (i == indexGraphics)
                    {
                        continue;
                    }

                    if ((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                        ((queueFamilies[i].queueFlags &
                          VK_QUEUE_GRAPHICS_BIT) == 0))
                    {
                        return i;
                    }
                }
            }

            // transfer only
            if ((flags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
            {
                for (std::uint32_t i = 0;
                     i < static_cast<std::uint32_t>(queueFamilies.size());
                     ++i)
                {
                    if (i == indexGraphics || i == indexCompute)
                    {
                        continue;
                    }

                    if ((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                        ((queueFamilies[i].queueFlags &
                          (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) == 0))
                    {
                        return i;
                    }
                }
            }

            for (std::uint32_t i = 0;
                 i < static_cast<std::uint32_t>(queueFamilies.size());
                 ++i)
            {
                if ((queueFamilies[i].queueFlags & flags) == flags)
                {
                    if (dedicated &&
                        ((queueFamilies[i].queueFlags &
                          VK_QUEUE_GRAPHICS_BIT) != 0) &&
                        (i == indexGraphics || i == indexCompute ||
                         i == indexTransfer))
                    {
                        continue; // skip graphics, compute, or transfer queues
                    }
                    return i;
                }
            }

            WS_ASSERT_MSG(
                false,
                "Failed to find a queue family with the requested flags");
            return std::numeric_limits<std::uint32_t>::max();
        }

        void detectQueueFamilyIndex(VkPhysicalDevice physicalDevice)
        {
            std::uint32_t queueFamilyCount{0};
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
                                                     &queueFamilyCount,
                                                     nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(
                queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
                                                     &queueFamilyCount,
                                                     queueFamilies.data());

            indexGraphics =
                getQueueFamilyIndex(queueFamilies, VK_QUEUE_GRAPHICS_BIT);
            indexCompute =
                getQueueFamilyIndex(queueFamilies, VK_QUEUE_COMPUTE_BIT);
            indexTransfer =
                getQueueFamilyIndex(queueFamilies, VK_QUEUE_TRANSFER_BIT);
        }

        void destroy()
        {
            regular.fill(nullptr);
            immediate.fill(nullptr);
        }
    } // namespace queues

    namespace descriptor
    {
        // global pool
        RHINativeHandle pool = {};

        void createPool()
        {
            // clang-format off
            std::array poolSizes = {
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER,                RHIConfig::MAX_DESCRIPTORS},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          RHIConfig::MAX_DESCRIPTORS},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          RHIConfig::MAX_DESCRIPTORS},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         RHIConfig::MAX_DESCRIPTORS},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, RHIConfig::MAX_DESCRIPTORS},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         RHIConfig::MAX_DESCRIPTORS},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, RHIConfig::MAX_DESCRIPTORS}};

            VkDescriptorPoolCreateInfo infoPool = {};
            infoPool.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            infoPool.flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
            infoPool.maxSets       = RHIConfig::MAX_DESCRIPTOR_SETS;
            infoPool.poolSizeCount = static_cast<std::uint32_t>(poolSizes.size());
            infoPool.pPoolSizes    = poolSizes.data();

            VkDescriptorPool vkPool = VK_NULL_HANDLE;
            WS_ASSERT_VK(vkCreateDescriptorPool(RHIContext::device,
                                                &infoPool,
                                                nullptr,
                                                &vkPool));
            pool = RHINativeHandle{vkPool, RHINativeHandleType::DescriptorPool};
            RHIDevice::setResourceName(pool, "global_descriptor_pool");
            // clang-format on
        }

        namespace global
        {
            RHINativeHandle layout;
            RHINativeHandle set;

            void createLayout()
            {
                // clang-format off
                VkDescriptorSetLayoutBinding layoutBindings[3] = {};

                // frame data
                layoutBindings[0].binding            = RHIConfig::FRAME_DATA_REGISTER;
                layoutBindings[0].descriptorCount    = 1;
                layoutBindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                layoutBindings[0].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
                layoutBindings[0].pImmutableSamplers = nullptr;

                // SamplerComparisonState
                layoutBindings[1].binding            = RHIConfig::SAMPLER_COMPARISON_STATE_REGISTER;
                layoutBindings[1].descriptorCount    = 1;
                layoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLER;
                layoutBindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
                layoutBindings[1].pImmutableSamplers = nullptr;
                
                // SamplerState
                layoutBindings[2].binding            = RHIConfig::SAMPLER_REGULAR_STATE_REGISTER;
                layoutBindings[2].descriptorCount    = 8;
                layoutBindings[2].descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLER;
                layoutBindings[2].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
                layoutBindings[2].pImmutableSamplers = nullptr;

                VkDescriptorBindingFlags flags[3] = {};
                flags[0] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                flags[1] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                flags[2] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

                VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags = {};
                bindingFlags.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                bindingFlags.bindingCount  = 3;
                bindingFlags.pBindingFlags = flags;

                VkDescriptorSetLayoutCreateInfo infoLayout = {};
                infoLayout.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                infoLayout.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
                infoLayout.pNext        = &bindingFlags;
                infoLayout.bindingCount = 3;
                infoLayout.pBindings    = layoutBindings;

                VkDescriptorSetLayout vkLayout = VK_NULL_HANDLE;
                WS_ASSERT_VK(vkCreateDescriptorSetLayout(RHIContext::device,
                                                         &infoLayout,
                                                         nullptr,
                                                         &vkLayout));
                layout = RHINativeHandle{vkLayout, RHINativeHandleType::DescriptorSetLayout};
                RHIDevice::setResourceName(layout, "global_descriptor_set_layout");
                // clang-format on
            }

            void allocateSet()
            {
                // clang-format off
                VkDescriptorSetLayout vkLayout = layout.asValue<VkDescriptorSetLayout>();

                VkDescriptorSetAllocateInfo infoAlloc = {};
                infoAlloc.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                infoAlloc.descriptorPool     = pool.asValue<VkDescriptorPool>();
                infoAlloc.descriptorSetCount = 1;
                infoAlloc.pSetLayouts        = &vkLayout;

                VkDescriptorSet vkSet = VK_NULL_HANDLE;
                WS_ASSERT_VK(vkAllocateDescriptorSets(RHIContext::device,
                                                       &infoAlloc,
                                                       &vkSet));
                set = RHINativeHandle{vkSet, RHINativeHandleType::DescriptorSet};
                RHIDevice::setResourceName(set, "global_descriptor_set");
                // clang-format on
            }

            void updateSet()
            {
                // create at first use
                if (!layout)
                {
                    createLayout();
                    allocateSet();
                }

                RHIBuffer* buffer =
                    RHIDevice::getResourceProvider()->getFrameConstantBuffer();

                // clang-format off
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = buffer->getHandle().asValue<VkBuffer>();
                bufferInfo.offset = 0;
                bufferInfo.range  = buffer->getSize();

                VkWriteDescriptorSet write[3] = {};

                // frame data
                write[0].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[0].dstSet           = set.asValue<VkDescriptorSet>();
                write[0].dstBinding       = RHIConfig::FRAME_DATA_REGISTER;
                write[0].descriptorCount  = 1;
                write[0].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write[0].pBufferInfo      = &bufferInfo;

                EnumArray<RHISamplerType, RHISampler*> samplers = RHIDevice::getResourceProvider()->getSamplers();
                std::array<VkDescriptorImageInfo, RHIConfig::MAX_RENDER_TARGET> samplerInfos = {};

                VkDescriptorImageInfo infoSamplerComparison = {};
                infoSamplerComparison.sampler  = samplers[RHISamplerType::CompareDepth]->getHandle().asValue<VkSampler>();

                // SamplerComparisonState
                write[1].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[1].dstSet           = set.asValue<VkDescriptorSet>();
                write[1].dstBinding       = RHIConfig::SAMPLER_COMPARISON_STATE_REGISTER;
                write[1].descriptorCount  = 1;
                write[1].descriptorType   = VK_DESCRIPTOR_TYPE_SAMPLER;
                write[1].pImageInfo       = &infoSamplerComparison;
                
                VkSampler samplerRegulars[8] = {
                    samplers[RHISamplerType::PointClampEdge]->getHandle().asValue<VkSampler>(),
                    samplers[RHISamplerType::PointClampBorder]->getHandle().asValue<VkSampler>(),
                    samplers[RHISamplerType::Wrap]->getHandle().asValue<VkSampler>(),
                    samplers[RHISamplerType::BilinearClampEdge]->getHandle().asValue<VkSampler>(),
                    samplers[RHISamplerType::BilinearClampBorder]->getHandle().asValue<VkSampler>(),
                    samplers[RHISamplerType::BilinearWrap]->getHandle().asValue<VkSampler>(),
                    samplers[RHISamplerType::TrilinearClamp]->getHandle().asValue<VkSampler>(),
                    samplers[RHISamplerType::AnisotropicClamp]->getHandle().asValue<VkSampler>()
                };
                std::array<VkDescriptorImageInfo, 8> infoSamplerRegulars = {};
                for (std::size_t i = 0; i < infoSamplerRegulars.size(); ++i)
                {
                    infoSamplerRegulars[i].sampler = samplerRegulars[i];
                }

                // SamplerState
                write[2].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write[2].dstSet           = set.asValue<VkDescriptorSet>();
                write[2].dstBinding       = RHIConfig::SAMPLER_REGULAR_STATE_REGISTER;
                write[2].descriptorCount  = 8;
                write[2].descriptorType   = VK_DESCRIPTOR_TYPE_SAMPLER;
                write[2].pImageInfo       = infoSamplerRegulars.data();

                vkUpdateDescriptorSets(RHIContext::device,
                                       3,
                                       write,
                                       0,
                                       nullptr);
                // clang-format on
            }
        } // namespace global

        namespace bindless
        {
            EnumArray<RHIBindlessResourceType, RHINativeHandle> layouts;
            EnumArray<RHIBindlessResourceType, RHINativeHandle> sets;

            // create bindless descriptor set layout
            void createLayout(RHIBindlessResourceType const type,
                              std::uint32_t const binding,
                              std::string_view name)
            {
                // clang-format off
                VkDescriptorSetLayoutBinding layoutBinding = {};
                layoutBinding.binding            = binding;
                layoutBinding.descriptorCount    = RHIConfig::MAX_DESCRIPTOR_SET_BINDINGS;
                layoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT |
                                                   VK_SHADER_STAGE_FRAGMENT_BIT |
                                                   VK_SHADER_STAGE_COMPUTE_BIT;
                layoutBinding.pImmutableSamplers = nullptr;

                if (type == RHIBindlessResourceType::MaterialTexture)
                {
                    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                }
                else if ((type == RHIBindlessResourceType::Material) ||
                         (type == RHIBindlessResourceType::Light))
                {
                    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                }
                else
                {
                    WS_ASSERT_MSG(false, "Unknown bindless resource type");
                }

                VkDescriptorBindingFlags bindingFlags = {};
                bindingFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                               VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                               VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

                VkDescriptorSetLayoutBindingFlagsCreateInfo infoBindingFlags = {};
                infoBindingFlags.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                infoBindingFlags.bindingCount  = 1;
                infoBindingFlags.pBindingFlags = &bindingFlags;

                VkDescriptorSetLayoutCreateInfo infoLayout = {};
                infoLayout.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                infoLayout.pNext        = &bindingFlags;
                infoLayout.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
                infoLayout.bindingCount = 1;
                infoLayout.pBindings    = &layoutBinding;

                VkDescriptorSetLayout vkSetLayout = VK_NULL_HANDLE;
                WS_ASSERT_VK(vkCreateDescriptorSetLayout(RHIContext::device,
                                                         &infoLayout,
                                                         nullptr,
                                                         &vkSetLayout));
                layouts[type] = RHINativeHandle{vkSetLayout, RHINativeHandleType::DescriptorSetLayout};
                RHIDevice::setResourceName(layouts[type], name);
                // clang-format on
            }

            // allocate bindless descriptor set
            // @param count number used in set, consume this amount of
            // descriptors from pool
            void allocateSet(RHIBindlessResourceType const type,
                             std::uint32_t const count, std::string_view name)
            {
                // clang-format off
                VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo = {};
                variableCountInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                variableCountInfo.descriptorSetCount = 1;
                variableCountInfo.pDescriptorCounts  = &count;

                VkDescriptorSetLayout vkSetLayout = layouts[type].asValue<VkDescriptorSetLayout>();

                VkDescriptorSetAllocateInfo infoAlloc = {};
                infoAlloc.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                infoAlloc.pNext              = &variableCountInfo;
                infoAlloc.descriptorPool     = pool.asValue<VkDescriptorPool>();
                infoAlloc.descriptorSetCount = 1;
                infoAlloc.pSetLayouts        = &vkSetLayout;

                VkDescriptorSet vkSet = VK_NULL_HANDLE;
                WS_ASSERT_VK(vkAllocateDescriptorSets(RHIContext::device,
                                                       &infoAlloc,
                                                       &vkSet));
                sets[type] = RHINativeHandle{vkSet, RHINativeHandleType::DescriptorSet};
                RHIDevice::setResourceName(sets[type], name);
                // clang-format on
            }

            void updateSet(RHIBindlessResourceType const type,
                           std::uint32_t const slot,
                           std::span<RHIDescriptorResource const> data,
                           std::string_view name)
            {
                // when compiing HLSL to SPIR-V, all the register are place in
                // the same register space by default. this will cause
                // conflicting bindings in vulkan, so we manually specify offset
                // for different types
                std::uint32_t binding = +slot;

                // create at first use
                if (!layouts[type])
                {
                    createLayout(type, binding, name);
                    allocateSet(type, data.size(), name);
                }

                // clang-format off
                if (type == RHIBindlessResourceType::MaterialTexture)
                {
                    std::vector<VkDescriptorImageInfo> imageInfos(data.size());

                    for (std::size_t i = 0; i < data.size(); ++i)
                    {
                        RHITexture const* texture = data[i].texture;
                        texture = texture ? texture : RHIDevice::getResourceProvider()->getPlaceholderTexture();

                        imageInfos[i].imageView = texture->getRtv().asValue<VkImageView>();
                        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    }

                    VkWriteDescriptorSet write = {};
                    write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet          = sets[type].asValue<VkDescriptorSet>();
                    write.dstBinding      = binding;
                    write.dstArrayElement = 0; // start index of the array
                    write.descriptorCount = static_cast<std::uint32_t>(data.size());
                    write.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                    write.pImageInfo      = imageInfos.data();

                    vkUpdateDescriptorSets(RHIContext::device, 1, &write, 0, nullptr);
                    // clang-format on
                }
                else if ((type == RHIBindlessResourceType::Material) ||
                         (type == RHIBindlessResourceType::Light))
                {
                    RHIBuffer* buffer = data[0].buffer;

                    VkDescriptorBufferInfo bufferInfo = {};
                    bufferInfo.buffer = buffer->getHandle().asValue<VkBuffer>();
                    bufferInfo.offset = 0;
                    bufferInfo.range  = buffer->getSize();

                    // clang-format off
                    VkWriteDescriptorSet write = {};
                    write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet          = sets[type].asValue<VkDescriptorSet>();
                    write.dstBinding      = binding;
                    write.dstArrayElement = 0;
                    write.descriptorCount = 1;
                    write.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    write.pBufferInfo     = &bufferInfo;

                    vkUpdateDescriptorSets(RHIContext::device, 1, &write, 0, nullptr);
                    // clang-format on
                }
            }
        } // namespace bindless

        namespace specific
        {
            // clang-format off
            std::unordered_map<std::uint64_t, std::shared_ptr<RHIDescriptorSetLayout>> layouts;
            std::unordered_map<std::uint64_t, RHIDescriptorSet>                        sets;
            std::unordered_map<std::uint64_t, std::vector<RHIDescriptor>>              descriptorCombinationCache;
            // clang-format on

            namespace detail
            {
                // find descriptors bound to the same slot, merge their
                // stageFlags
                void
                mergeDescriptors(std::vector<RHIDescriptor>& bases,
                                 std::vector<RHIDescriptor> const& additionals)
                {
                    for (RHIDescriptor const& additional : additionals)
                    {
                        auto it = std::find_if(
                            bases.begin(),
                            bases.end(),
                            [slot = additional.slot](RHIDescriptor const& base)
                            {
                                return base.slot == slot;
                            });

                        if (it != bases.end())
                        {
                            it->stageFlags |= additional.stageFlags;
                        }
                        else
                        {
                            bases.push_back(additional);
                        }
                    }
                }
            } // namespace detail

            // collect descriptors from pipeline shaders
            void getDescriptorFromPipelineState(
                RHIPipelineState const& pso,
                std::vector<RHIDescriptor>& descriptors)
            {
                std::uint64_t hash = pso.getHash();
                auto it            = descriptorCombinationCache.find(hash);
                if (it != descriptorCombinationCache.end())
                {
                    descriptors = it->second;
                    return;
                }

                descriptors.clear();
                if (pso.type == RHIPipelineType::Compute)
                {
                    descriptors =
                        pso.getShader(RHIShaderType::Compute)->getDescriptors();
                }
                else if (pso.type == RHIPipelineType::Graphics)
                {
                    descriptors =
                        pso.getShader(RHIShaderType::Vertex)->getDescriptors();
                    detail::mergeDescriptors(
                        descriptors,
                        pso.getShader(RHIShaderType::Pixel)->getDescriptors());
                }

                std::sort(descriptors.begin(),
                          descriptors.end(),
                          [](RHIDescriptor const& a, RHIDescriptor const& b)
                          {
                              return a.slot < b.slot;
                          });

                // cache result
                descriptorCombinationCache.emplace(hash, descriptors);
            }

            // get or create dynamic descriptor set layout
            RHIDescriptorSetLayout*
            getOrCreateDescriptorSetLayout(RHIPipelineState const& pso)
            {
                std::vector<RHIDescriptor> descriptors;
                getDescriptorFromPipelineState(pso, descriptors);

                // calculate hash of the descriptors
                std::uint64_t hash = 0;
                for (RHIDescriptor const& descriptor : descriptors)
                {
                    hash = math::hashCombine(
                        hash,
                        static_cast<std::uint64_t>(descriptor.slot));
                    hash = math::hashCombine(
                        hash,
                        static_cast<std::uint64_t>(descriptor.stageFlags));
                }

                auto it     = layouts.find(hash);
                bool cached = it != layouts.end();

                if (!cached)
                {
                    it = layouts
                             .emplace(hash,
                                      std::make_shared<RHIDescriptorSetLayout>(
                                          descriptors,
                                          pso.name))
                             .first;
                }

                // TODO: support this
                // if (cached)
                // {
                //     it->second->clearData();
                // }

                return it->second.get();
            }
        } // namespace specific

        // TODO: move away from descriptor
        std::mutex mtxPipelines;
        std::unordered_map<std::uint64_t, std::shared_ptr<RHIPipeline>>
            pipelines;

        void release()
        {
            RHIDevice::deletionQueueAdd(pool);

            // global
            RHIDevice::deletionQueueAdd(global::layout);

            // bindless
            for (RHINativeHandle const& layout : bindless::layouts)
            {
                RHIDevice::deletionQueueAdd(layout);
            }

            // specific
            specific::layouts.clear();
            specific::sets.clear();
            specific::descriptorCombinationCache.clear();

            pipelines.clear();
        }

    } // namespace descriptor

    namespace vma
    {
        std::mutex mtxAllocator;
        VmaAllocator allocator = VK_NULL_HANDLE;

        struct AllocationData
        {
            VmaAllocation allocation;
            RHINativeHandle handle;
        };

        std::mutex mtxAllocation;
        std::unordered_map<std::uint64_t, AllocationData> allocations;

        void create()
        {
            VmaAllocatorCreateInfo infoAllocator{};
            infoAllocator.physicalDevice   = RHIContext::physicalDevice;
            infoAllocator.device           = RHIContext::device;
            infoAllocator.instance         = RHIContext::instance;
            infoAllocator.vulkanApiVersion = config::version;

            VmaVulkanFunctions vulkanFunctions{};
            infoAllocator.pVulkanFunctions = &vulkanFunctions;
            WS_ASSERT_VK(vmaImportVulkanFunctionsFromVolk(&infoAllocator,
                                                          &vulkanFunctions))

            WS_ASSERT_VK(vmaCreateAllocator(&infoAllocator, &vma::allocator));
        }

        void destroy()
        {
            if (!allocations.empty())
            {
                WS_LOG_WARN("VMA",
                            "There are still {} allocations in VMA",
                            allocations.size());
                for (auto const& [handle, allocation] : allocations)
                {
                    WS_LOG_WARN("VMA", "Allocation: {}", handle);
                }
            }

            vmaDestroyAllocator(vma::allocator);
            vma::allocator = VK_NULL_HANDLE;
        }

        void saveAllocation(VmaAllocation const& allocation,
                            RHINativeHandle handle)
        {
            WS_ASSERT(handle);

            std::lock_guard lock{mtxAllocation};
            allocations.emplace(handle.asValue(),
                                AllocationData{allocation, handle});
        }

        // thread safe
        AllocationData* getAllocation(RHINativeHandle handle)
        {
            WS_ASSERT(handle);
            std::lock_guard lock{mtxAllocation};
            if (auto it = allocations.find(handle.asValue());
                it != allocations.end())
            {
                return &it->second;
            }
            else
            {
                return nullptr;
            }
        }

        // thread safe
        void removeAllocation(RHINativeHandle handle)
        {
            WS_ASSERT(handle);
            std::lock_guard lock{mtxAllocation};
            allocations.erase(handle.asValue());
        }
    } // namespace vma

    namespace
    {
        RHIResourceProvider* resourceProvider = nullptr;

        std::mutex mtxDeletionQueue;
        std::unordered_map<RHINativeHandleType, std::vector<RHINativeHandle>>
            deletionQueue;
    } // namespace

    void RHIDevice::initialize()
    {
        // instance
        {
            VkApplicationInfo infoApp = {};
            infoApp.sType             = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            infoApp.apiVersion        = config::version;

            // clang-format off
            VkInstanceCreateInfo infoInst    = {};
            infoInst.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            infoInst.pNext                   = &debugMessenger::info;
            infoInst.flags                   = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            infoInst.pApplicationInfo        = &infoApp;
            infoInst.enabledLayerCount       = RHIConfig::enableValidationLayers ? 1u : 0u;
            infoInst.ppEnabledLayerNames     = &validation::name;

            std::vector<char const*> extensionsInst = extensions::getExtensionsInstance();
            infoInst.enabledExtensionCount   = static_cast<std::uint32_t>(extensionsInst.size());
            infoInst.ppEnabledExtensionNames = extensionsInst.data();
            // clang-format on

            WS_ASSERT_VK(
                vkCreateInstance(&infoInst, nullptr, &RHIContext::instance));

            volkLoadInstance(RHIContext::instance);

            if (RHIConfig::enableValidationLayers)
            {
                debugMessenger::create();
            }
        }

        // device
        {
            physicalDevice::select();

            // queues
            std::vector<VkDeviceQueueCreateInfo> queueInfos;
            float queuePriority{1.0f};
            {
                queues::detectQueueFamilyIndex(RHIContext::physicalDevice);
                std::vector queueFamilyIndices{
                    queues::indexGraphics,
                    queues::indexCompute,
                    queues::indexTransfer,
                };

                for (auto const& index : queueFamilyIndices)
                {
                    // clang-format off
                    VkDeviceQueueCreateInfo infoQueue = {};
                    infoQueue.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    infoQueue.queueFamilyIndex = index;
                    infoQueue.queueCount       = 1;
                    infoQueue.pQueuePriorities = &queuePriority;
                    // clang-format on
                    queueInfos.emplace_back(infoQueue);
                };
            }

            deviceFeatures::detect();

            // clang-format off
            VkDeviceCreateInfo infoDevice      = {};
            infoDevice.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            infoDevice.pNext                   = deviceFeatures::featureChain;
            infoDevice.queueCreateInfoCount    = static_cast<std::uint32_t>(queueInfos.size());
            infoDevice.pQueueCreateInfos       = queueInfos.data();

            std::vector<char const*> extensionsDevice = extensions::getExtensionsDevice();
            infoDevice.enabledExtensionCount   = static_cast<std::uint32_t>(extensionsDevice.size());
            infoDevice.ppEnabledExtensionNames = extensionsDevice.data();
            // clang-format on

            WS_ASSERT_VK(vkCreateDevice(RHIContext::physicalDevice,
                                        &infoDevice,
                                        nullptr,
                                        &RHIContext::device));
            RHIContext::backendType = RHIBackendType::Vulkan;

            volkLoadDevice(RHIContext::device);

            // version
            {
                VkPhysicalDeviceProperties physicalDeviceProperties;
                vkGetPhysicalDeviceProperties(RHIContext::physicalDevice,
                                              &physicalDeviceProperties);
                // clang-format off
                WS_LOG_INFO("RHI Backend", "Vulkan {}.{}.{}", 
                    std::to_string(VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion)),
                    std::to_string(VK_VERSION_MINOR(physicalDeviceProperties.apiVersion)),
                    std::to_string(VK_VERSION_PATCH(physicalDeviceProperties.apiVersion)));
                // clang-format on
            }
        }

        // queues
        {
            // clang-format off
            VkQueue queue = VK_NULL_HANDLE;
            vkGetDeviceQueue(RHIContext::device, queues::indexGraphics, 0, &queue);
            queues::graphics = RHINativeHandle{queue, RHINativeHandleType::Queue};
            RHIDevice::setResourceName(queues::graphics, "graphics");

            vkGetDeviceQueue(RHIContext::device, queues::indexCompute, 0, &queue);
            queues::compute = RHINativeHandle{queue, RHINativeHandleType::Queue};
            RHIDevice::setResourceName(queues::compute, "compute");

            vkGetDeviceQueue(RHIContext::device, queues::indexTransfer, 0, &queue);
            queues::transfer = RHINativeHandle{queue, RHINativeHandleType::Queue};
            RHIDevice::setResourceName(queues::transfer, "transfer");

            queues::regular[static_cast<std::size_t>(RHIQueueType::Graphics)] = std::make_shared<RHIQueue>(RHIQueueType::Graphics, "graphics");
            queues::regular[static_cast<std::size_t>(RHIQueueType::Compute)]  = std::make_shared<RHIQueue>(RHIQueueType::Compute, "compute");
            queues::regular[static_cast<std::size_t>(RHIQueueType::Transfer)] = std::make_shared<RHIQueue>(RHIQueueType::Transfer, "transfer");

            queues::immediate[static_cast<std::size_t>(RHIQueueType::Graphics)] = std::make_shared<RHIQueue>(RHIQueueType::Graphics, "graphics");
            queues::immediate[static_cast<std::size_t>(RHIQueueType::Compute)]  = std::make_shared<RHIQueue>(RHIQueueType::Compute, "compute");
            queues::immediate[static_cast<std::size_t>(RHIQueueType::Transfer)] = std::make_shared<RHIQueue>(RHIQueueType::Transfer, "transfer");
            // clang-format on
        }

        vma::create();
        descriptor::createPool();

        DXCompiler::initialize();
    }

    void RHIDevice::destroy()
    {
        RHIDevice::queueWaitAll();
        queues::destroy();

        descriptor::release();

        RHIDevice::deletionQueueFlush();

        DXCompiler::shutdown();
        vma::destroy();

        vkDestroyDevice(RHIContext::device, nullptr);

        if (RHIConfig::enableValidationLayers)
        {
            debugMessenger::destory();
        }

        vkDestroyInstance(RHIContext::instance, nullptr);

        volkFinalize();
    }

    void RHIDevice::setResourceProvider(RHIResourceProvider* provider)
    {
        WS_ASSERT(provider);
        WS_ASSERT(provider->validate());
        resourceProvider = provider;
    }

    RHIResourceProvider* RHIDevice::getResourceProvider()
    {
        return resourceProvider;
    }

    void RHIDevice::queueWaitAll()
    {
        queues::regular[static_cast<std::size_t>(RHIQueueType::Graphics)]
            ->wait();
        queues::regular[static_cast<std::size_t>(RHIQueueType::Compute)]
            ->wait();
    }

    std::uint32_t RHIDevice::getQueueIndex(RHIQueueType const type)
    {
        if (type == RHIQueueType::Graphics)
        {
            return queues::indexGraphics;
        }
        if (type == RHIQueueType::Compute)
        {
            return queues::indexCompute;
        }
        if (type == RHIQueueType::Transfer)
        {
            return queues::indexTransfer;
        }

        return 0;
    }

    RHIQueue* RHIDevice::getQueue(RHIQueueType const type)
    {
        if (type == RHIQueueType::Graphics)
        {
            return queues::regular[static_cast<std::size_t>(
                                       RHIQueueType::Graphics)]
                .get();
        }
        if (type == RHIQueueType::Compute)
        {
            return queues::regular[static_cast<std::size_t>(
                                       RHIQueueType::Compute)]
                .get();
        }

        return nullptr;
    }

    RHINativeHandle RHIDevice::getQueueRHIResource(RHIQueueType const type)
    {
        if (type == RHIQueueType::Graphics)
        {
            return queues::graphics;
        }
        if (type == RHIQueueType::Compute)
        {
            return queues::compute;
        }
        if (type == RHIQueueType::Transfer)
        {
            return queues::transfer;
        }

        return RHINativeHandle{};
    }

    // allocate generic set from global pool
    RHINativeHandle
    RHIDevice::allocateDescriptorSet(RHIDescriptorSetLayout const& layout)
    {
        // clang-format off
        VkDescriptorSetLayout vkLayout = layout.getHandle().asValue<VkDescriptorSetLayout>();

        VkDescriptorSetAllocateInfo infoAlloc = {};
        infoAlloc.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        infoAlloc.descriptorPool     = descriptor::pool.asValue<VkDescriptorPool>();
        infoAlloc.descriptorSetCount = 1;
        infoAlloc.pSetLayouts        = &vkLayout;

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        WS_ASSERT_VK(vkAllocateDescriptorSets(RHIContext::device,
                                              &infoAlloc,
                                              &descriptorSet));
        RHINativeHandle handle{descriptorSet, RHINativeHandleType::DescriptorSet};
        // clang-format on
        return handle;
    }

    RHINativeHandle RHIDevice::getGlobalDescriptorSetLayout()
    {
        return descriptor::global::layout;
    }

    RHINativeHandle RHIDevice::getGlobalDescriptorSet()
    {
        return descriptor::global::set;
    }

    std::unordered_map<std::uint64_t, RHIDescriptorSet>&
    RHIDevice::getDescriptorSets()
    {
        return descriptor::specific::sets;
    }

    RHINativeHandle
    RHIDevice::getBindlessDescriptorSet(RHIBindlessResourceType const type)
    {
        return descriptor::bindless::sets[type];
    }

    RHINativeHandle RHIDevice::getBindlessDescriptorSetLayout(
        RHIBindlessResourceType const type)
    {
        return descriptor::bindless::layouts[type];
    }

    void RHIDevice::updateGlobalDescriptorSet()
    {
        descriptor::global::updateSet();
    }

    void RHIDevice::updateBindlessResources()
    {

        // descriptor::bindless::updateSet(
        //     RHIBindlessResourceType::MaterialTexture,
        //     RHIConfig::MATERIAL_TEXTURE_SLOT,
        //     {},
        //     "material_texture");

        // descriptor::bindless::updateSet(RHIBindlessResourceType::Material,
        //                                 RHIConfig::MATERIAL_PROPERTIES_SLOT,
        //                                 {},
        //                                 "material_properties");

        // descriptor::bindless::updateSet(RHIBindlessResourceType::Light,
        //                                 RHIConfig::LIGHT_PROPERTIES_SLOT,
        //                                 {},
        //                                 "light_properties");
    }

    void
    RHIDevice::getOrCreatePipeline(RHIPipelineState const& pso,
                                   RHIPipeline*& pipeline,
                                   RHIDescriptorSetLayout*& descriptorSetLayout)
    {
        descriptorSetLayout =
            descriptor::specific::getOrCreateDescriptorSetLayout(pso);
        WS_ASSERT(descriptorSetLayout);

        std::lock_guard guard{descriptor::mtxPipelines};

        std::uint64_t hash = pso.getHash();
        auto it            = descriptor::pipelines.find(hash);
        if (it == descriptor::pipelines.end())
        {
            it = descriptor::pipelines
                     .emplace(
                         hash,
                         std::make_shared<RHIPipeline>(pso,
                                                       *descriptorSetLayout))
                     .first;
        }

        pipeline = it->second.get();
        WS_ASSERT(pipeline);
    }

    void RHIDevice::memoryTextureCreate(RHITexture* texture)
    {
        VkImageUsageFlags vkUsage  = 0;
        RHITextureUsageFlags usage = texture->getUsage();
        // clang-format off
        if (usage & RHITextureUsageFlagBits::Srv)         { vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT; }
        if (usage & RHITextureUsageFlagBits::Uav)         { vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT; }
        if (usage & RHITextureUsageFlagBits::Rtv)         { vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; }
        if (usage & RHITextureUsageFlagBits::Dsv)         { vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; }
        if (usage & RHITextureUsageFlagBits::ClearOrBlit) { vkUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; }

        VkImageCreateInfo infoImage = {};
        infoImage.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        infoImage.imageType     = texture->getType() == RHITextureType::Texture3D ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
        infoImage.format        = vulkanFormat(texture->getFormat());
        infoImage.extent.width  = texture->getWidth();
        infoImage.extent.height = texture->getHeight();
        infoImage.extent.depth  = texture->getType() == RHITextureType::Texture3D ? texture->getDepth() : 1;
        infoImage.mipLevels     = 1;
        infoImage.arrayLayers   = texture->getType() == RHITextureType::Texture3D ? 1 : texture->getDepth();
        infoImage.samples       = VK_SAMPLE_COUNT_1_BIT;
        infoImage.tiling        = VK_IMAGE_TILING_OPTIMAL;
        infoImage.usage         = vkUsage;
        infoImage.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
        infoImage.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // clang-format on

        VmaAllocationInfo infoAlloc = {};
        VmaAllocation allocation    = VK_NULL_HANDLE;

        VmaAllocationCreateInfo infoAllocCreate = {};
        infoAllocCreate.usage                   = VMA_MEMORY_USAGE_AUTO;
        infoAllocCreate.flags                   = 0;

        VkImage image   = VK_NULL_HANDLE;
        VkResult result = vmaCreateImage(vma::allocator,
                                         &infoImage,
                                         &infoAllocCreate,
                                         &image,
                                         &allocation,
                                         &infoAlloc);
        if ((result == VK_ERROR_OUT_OF_DEVICE_MEMORY) ||
            (result == VK_ERROR_OUT_OF_HOST_MEMORY))
        {
            WS_LOG_ERROR("Vulkan", "Allocation out of memory");
        }
        WS_ASSERT_VK(result);

        texture->m_image = RHINativeHandle{image, RHINativeHandleType::Image};
        vmaSetAllocationName(vma::allocator,
                             allocation,
                             texture->getName().c_str());
        RHIDevice::setResourceName(texture->getImage(), texture->getName());

        vma::saveAllocation(allocation, texture->getImage());
    }

    void RHIDevice::memoryTextureDestroy(RHINativeHandle handle)
    {
        vma::AllocationData* data = vma::getAllocation(handle);
        if (data)
        {
            {
                std::lock_guard guard{vma::mtxAllocator};
                vmaDestroyImage(vma::allocator,
                                handle.asValue<VkImage>(),
                                data->allocation);
            }
            vma::removeAllocation(handle);
        }
    }

    void RHIDevice::memoryBufferCreate(RHINativeHandle& buffer,
                                       std::uint32_t size,
                                       std::uint32_t bufferUsage,
                                       std::uint32_t memoryProperty,
                                       void const* data, std::string_view name)
    {
        // clang-format off
        VkBufferCreateInfo infoBuffer = {};
        infoBuffer.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        infoBuffer.size        = size;
        infoBuffer.usage       = bufferUsage;
        infoBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo infoAllocCreate = {};
        infoAllocCreate.usage         = VMA_MEMORY_USAGE_AUTO;
        infoAllocCreate.requiredFlags = memoryProperty;

        bool mappable = (memoryProperty & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
        if (mappable)
        {
            infoAllocCreate.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        VmaAllocationInfo infoAlloc = {};
        VmaAllocation allocation    = VK_NULL_HANDLE;

        VkBuffer vkBuffer = VK_NULL_HANDLE;
        VkResult result   = vmaCreateBuffer(vma::allocator,
                                          &infoBuffer,
                                          &infoAllocCreate,
                                          &vkBuffer,
                                          &allocation,
                                          &infoAlloc);
        if ((result == VK_ERROR_OUT_OF_DEVICE_MEMORY) ||
            (result == VK_ERROR_OUT_OF_HOST_MEMORY))
        {
            WS_LOG_ERROR("Vulkan", "Allocation out of memory");
        }
        WS_ASSERT_VK(result);

        buffer = RHINativeHandle{vkBuffer, RHINativeHandleType::Buffer};
        vmaSetAllocationName(vma::allocator,
                             allocation,
                             name.data());
        RHIDevice::setResourceName(buffer, name);

        if (data)
        {
            WS_ASSERT(mappable);

            void* mappedData = nullptr;
            // get mapped data pointer
            WS_ASSERT_VK(vmaMapMemory(vma::allocator, allocation, &mappedData));

            
            std::memcpy(mappedData, data, size);
            WS_ASSERT_VK(vmaFlushAllocation(vma::allocator,
                                            allocation,
                                            0,
                                            size));
            vmaUnmapMemory(vma::allocator, allocation);
        }

        vma::saveAllocation(allocation, buffer);
        // clang-format on
    }

    void RHIDevice::memoryBufferDestroy(RHINativeHandle handle)
    {
        vma::AllocationData* data = vma::getAllocation(handle);
        if (data)
        {
            {
                std::lock_guard guard{vma::mtxAllocator};
                vmaDestroyBuffer(vma::allocator,
                                 handle.asValue<VkBuffer>(),
                                 data->allocation);
            }
            vma::removeAllocation(handle);
        }
    }

    void* RHIDevice::memoryGetMappedBufferData(RHINativeHandle handle)
    {
        vma::AllocationData* data = vma::getAllocation(handle);
        if (data && data->allocation)
        {
            VmaAllocationInfo allocInfo;
            vmaGetAllocationInfo(vma::allocator, data->allocation, &allocInfo);
            return allocInfo.pMappedData;
        }
        return nullptr;
    }

    void RHIDevice::deletionQueueAdd(RHINativeHandle const& resource)
    {
        if (!resource)
        {
            return;
        }

        std::lock_guard guard{mtxDeletionQueue};
        deletionQueue[resource.getType()].emplace_back(resource);
    }

    void RHIDevice::deletionQueueFlush()
    {
        std::lock_guard guard{mtxDeletionQueue};

        for (auto& [type, handles] : deletionQueue)
        {
            for (auto& handle : handles)
            {
                // clang-format off
                switch (type)
                {
                case RHINativeHandleType::Image:               RHIDevice::memoryTextureDestroy(handle);                                                            break;
                case RHINativeHandleType::ImageView:           vkDestroyImageView(RHIContext::device, handle.asValue<VkImageView>(), nullptr);                     break;
                case RHINativeHandleType::Fence:               vkDestroyFence(RHIContext::device, handle.asValue<VkFence>(), nullptr);                             break;
                case RHINativeHandleType::Semaphore:           vkDestroySemaphore(RHIContext::device, handle.asValue<VkSemaphore>(), nullptr);                     break;
                case RHINativeHandleType::Shader:              vkDestroyShaderModule(RHIContext::device, handle.asValue<VkShaderModule>(), nullptr);               break;
                case RHINativeHandleType::Pipeline:            vkDestroyPipeline(RHIContext::device, handle.asValue<VkPipeline>(), nullptr);                       break;
                case RHINativeHandleType::PipelineLayout:      vkDestroyPipelineLayout(RHIContext::device, handle.asValue<VkPipelineLayout>(), nullptr);           break;
                case RHINativeHandleType::DescriptorPool:      vkDestroyDescriptorPool(RHIContext::device, handle.asValue<VkDescriptorPool>(), nullptr);           break;
                case RHINativeHandleType::DescriptorSetLayout: vkDestroyDescriptorSetLayout(RHIContext::device, handle.asValue<VkDescriptorSetLayout>(), nullptr); break;
                case RHINativeHandleType::Buffer:              RHIDevice::memoryBufferDestroy(handle);                                                             break;
                case RHINativeHandleType::Sampler:             vkDestroySampler(RHIContext::device, handle.asValue<VkSampler>(), nullptr);                         break;
                default:
                    WS_LOG_ERROR("RHI", "Unhandled handle type");
                    WS_ASSERT(false);
                }
                // clang-format on

                handle = {};
            }
        }

        // Clear the deletion queue after processing
        deletionQueue.clear();
    }

    RHICommandList* RHIDevice::cmdImmediateBegin(RHIQueueType const type)
    {
        std::lock_guard guard{queues::mtxImmediateCmd};
        // wait
        queues::semImmediateCmd.acquire();
        queues::isImmediateCmdActive = true;
        queues::activeQueue =
            queues::immediate[static_cast<std::size_t>(type)].get();

        RHICommandList* cmdList = queues::activeQueue->nextCommandList();
        cmdList->begin();

        return cmdList;
    }

    void RHIDevice::cmdImmediateSubmit(RHICommandList* cmdList)
    {
        cmdList->submit(nullptr);
        cmdList->waitForExecution();

        queues::isImmediateCmdActive = false;
        queues::activeQueue          = nullptr;
        queues::semImmediateCmd.release();
    }

    void RHIDevice::setResourceName(RHINativeHandle const& resource,
                                    std::string_view name)
    {
        if (!RHIConfig::enableValidationLayers)
        {
            return;
        }

        // clang-format off
        VkDebugUtilsObjectNameInfoEXT info{};
        info.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        info.objectType   = vulkanObjectType(resource.getType());
        info.objectHandle = resource.asValue<std::uint64_t>();
        info.pObjectName  = name.data();
        // clang-format on

        WS_ASSERT_VK(vkSetDebugUtilsObjectNameEXT(RHIContext::device, &info));
    }

} // namespace worse