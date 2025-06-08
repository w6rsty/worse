#include "Log.hpp"
#include "Math/Hash.hpp"
#include "RHIQueue.hpp"
#include "RHIDevice.hpp"
#include "RHIShader.hpp"
#include "RHITexture.hpp"
#include "RHIDescriptor.hpp"
#include "RHICommandList.hpp"
#include "RHIDescriptorSetLayout.hpp"
#include "Pipeline/RHIPipeline.hpp"
#include "Pipeline/RHIPipelineState.hpp"

#include "SDL3/SDL_vulkan.h"
#include "vk_mem_alloc.h"

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
        std::uint32_t version         = VK_API_VERSION_1_3;
        bool isValidationLayerEnabled = true;
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

            if (config::isValidationLayerEnabled)
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
            featureDescriptorIndexing.sType                                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
            featureDescriptorIndexing.runtimeDescriptorArray                    = VK_TRUE;
            featureDescriptorIndexing.descriptorBindingVariableDescriptorCount  = VK_TRUE;
            featureDescriptorIndexing.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
            featureDescriptorIndexing.pNext                                     = nullptr;

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
        // clang-format off
        // global pool
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

        std::mutex mtxPipelines;
        std::unordered_map<std::uint64_t, std::shared_ptr<RHIPipeline>>
            pipelines;
        std::unordered_map<std::uint64_t, std::shared_ptr<RHIDescriptorSetLayout>>
            descriptorSetLayouts;
        std::unordered_map<std::uint64_t, std::vector<RHIDescriptor>>
            piplineDescriptorCombinations;
        // clang-format on

        void createPool()
        {
            // clang-format off
            std::array poolSizes = {
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER,                512},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          512},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          512},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         512},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 512},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 512}
            };
            
            VkDescriptorPoolCreateInfo infoPool = {};
            infoPool.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            infoPool.flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
            infoPool.maxSets       = 512;
            infoPool.poolSizeCount = static_cast<std::uint32_t>(poolSizes.size());
            infoPool.pPoolSizes    = poolSizes.data();
            // clang-format on

            WS_ASSERT_VK(vkCreateDescriptorPool(RHIContext::device,
                                                &infoPool,
                                                nullptr,
                                                &descriptor::descriptorPool));
        }

        void mergeDescriptors(std::vector<RHIDescriptor>& bases,
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

        // extract descriptors from pipeline shaders
        void
        getDescriptorFromPipelineState(RHIPipelineState const& pso,
                                       std::vector<RHIDescriptor>& descriptors)
        {
            // clang-format off
            std::uint64_t hash = pso.getHash();
            auto it = descriptor::piplineDescriptorCombinations.find(hash);
            if (it != descriptor::piplineDescriptorCombinations.end())
            {
                descriptors = it->second;
                return;
            }

            descriptors.clear();
            if (pso.type == RHIPipelineType::Compute)
            {
                descriptors = pso.getShader(RHIShaderType::Compute)->getDescriptors();
            }
            else if (pso.type == RHIPipelineType::Graphics)
            {
                descriptors = pso.getShader(RHIShaderType::Vertex)->getDescriptors();
                mergeDescriptors(descriptors, pso.getShader(RHIShaderType::Pixel)->getDescriptors());
            }

            std::sort(
                descriptors.begin(),
                descriptors.end(),
                [](RHIDescriptor const& a, RHIDescriptor const& b) { return a.slot < b.slot; }
            );

            // cache result
            descriptor::piplineDescriptorCombinations.emplace(hash, descriptors);
            // clang-format on
        }

        RHIDescriptorSetLayout*
        getOrCreateDescriptorSetLayout(RHIPipelineState const& pso)
        {
            // clang-format off
            std::vector<RHIDescriptor> descriptors;
            getDescriptorFromPipelineState(pso, descriptors);

            // calculate hash of the descriptors
            std::uint64_t hash = 0;
            for (RHIDescriptor const& descriptor : descriptors)
            {
                hash = math::hashCombine(hash, static_cast<std::uint64_t>(descriptor.slot));
                hash = math::hashCombine(hash, static_cast<std::uint64_t>(descriptor.stageFlags));
            }

            auto it     = descriptor::descriptorSetLayouts.find(hash);
            bool cached = it != descriptor::descriptorSetLayouts.end();

            if (!cached)
            {
                it = descriptor::descriptorSetLayouts.emplace(
                    hash,
                    std::make_shared<RHIDescriptorSetLayout>(descriptors, pso.name)
                ).first;
            }

            // TODO: support this
            // if (cached)
            // {
            //     it->second->clearData();
            // }

            return it->second.get();
            // clang-format on
        }

        void release()
        {
            pipelines.clear();
            descriptorSetLayouts.clear();
            piplineDescriptorCombinations.clear();
        }

        namespace bindless
        {
            std::array<RHINativeHandle, k_rhiBindlessResourceCount>
                descriptorSetLayouts;
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

        void removeAllocation(RHINativeHandle handle)
        {
            WS_ASSERT(handle);
            std::lock_guard lock{mtxAllocation};
            allocations.erase(handle.asValue());
        }
    } // namespace vma

    namespace
    {
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
            infoInst.enabledLayerCount       = config::isValidationLayerEnabled ? 1u : 0u;
            infoInst.ppEnabledLayerNames     = &validation::name;

            std::vector<char const*> extensionsInst = extensions::getExtensionsInstance();
            infoInst.enabledExtensionCount   = static_cast<std::uint32_t>(extensionsInst.size());
            infoInst.ppEnabledExtensionNames = extensionsInst.data();
            // clang-format on

            WS_ASSERT_VK(
                vkCreateInstance(&infoInst, nullptr, &RHIContext::instance));

            volkLoadInstance(RHIContext::instance);

            if (config::isValidationLayerEnabled)
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
    }

    void RHIDevice::destroy()
    {
        RHIDevice::queueWaitAll();
        queues::destroy();

        vkDestroyDescriptorPool(RHIContext::device,
                                descriptor::descriptorPool,
                                nullptr);
        descriptor::descriptorPool = VK_NULL_HANDLE;
        descriptor::release();

        RHIDevice::deletionQueueFlush();

        vma::destroy();

        vkDestroyDevice(RHIContext::device, nullptr);

        if (config::isValidationLayerEnabled)
        {
            debugMessenger::destory();
        }

        vkDestroyInstance(RHIContext::instance, nullptr);

        volkFinalize();
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

    RHINativeHandle RHIDevice::getBindlessDescriptorSetLayout(
        RHIBindlessResourceType const type)
    {
        return descriptor::bindless::descriptorSetLayouts
            [static_cast<std::size_t>(type)];
    }

    void
    RHIDevice::getOrCreatePipeline(RHIPipelineState const& pso,
                                   RHIPipeline*& pipeline,
                                   RHIDescriptorSetLayout*& descriptorSetLayout)
    {
        descriptorSetLayout = descriptor::getOrCreateDescriptorSetLayout(pso);
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

        texture->setHandle(RHINativeHandle{image, RHINativeHandleType::Image});
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
                case RHINativeHandleType::DescriptorSetLayout: vkDestroyDescriptorSetLayout(RHIContext::device, handle.asValue<VkDescriptorSetLayout>(), nullptr); break;
                default:
                    // TODO
                    WS_LOG_ERROR("RHI", "Unhandled handle type {}", 
                                 static_cast<std::size_t>(type));
                    WS_ASSERT(false);
                }
                // clang-format on

                handle = {};
            }
        }

        // Clear the deletion queue after processing
        deletionQueue.clear();
    }

    RHICommandList* RHIDevice::CmdImmediateBegin(RHIQueueType const type)
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

    void RHIDevice::CmdImmediateSubmit(RHICommandList* cmdList)
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
        if (!config::isValidationLayerEnabled)
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