#include "RHIDevice.hpp"
#include "RHIResource.hpp"
#include "VulkanDescriptor.hpp"

namespace worse
{

    VkDescriptorPool VulkanDescriptorAllocator::createPool()
    {
        std::uint32_t count = m_expandRatio * RHIConfig::MIN_DESCRIPTORS;
        if (count < RHIConfig::MAX_DESCRIPTORS)
        {
            m_expandRatio *= 2;
        }

        // clang-format off
        std::array poolSizes = {
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER,                count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         count},
        };

        VkDescriptorPoolCreateInfo infoPool = {};
        infoPool.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        infoPool.flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        infoPool.maxSets       = count;
        infoPool.poolSizeCount = static_cast<std::uint32_t>(poolSizes.size());
        infoPool.pPoolSizes    = poolSizes.data();
        
        VkDescriptorPool vkPool = VK_NULL_HANDLE;
        WS_ASSERT_VK(vkCreateDescriptorPool(RHIContext::device, &infoPool, nullptr, &vkPool));
        
        m_pools[m_rotateIndex].push_back(vkPool);

        std::string name = "descriptor_pool_" + std::to_string(m_pools.size() - 1);
        RHIDevice::setResourceName(RHINativeHandle{vkPool, RHINativeHandleType::DescriptorPool}, name);

        return vkPool;
        // clang-format on
    }

    RHINativeHandle
    VulkanDescriptorAllocator::allocateInternal(VkDescriptorSetLayout layout,
                                                void* pNext)
    {
        if (m_pools[m_rotateIndex].empty())
        {
            createPool();
            m_currentPoolIndex[m_rotateIndex] = 0;
        }

        VkDescriptorSetAllocateInfo infoAlloc = {};
        infoAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        infoAlloc.pNext = pNext;
        infoAlloc.descriptorSetCount = 1;
        infoAlloc.pSetLayouts        = &layout;

        VkDescriptorSet set = VK_NULL_HANDLE;
        VkResult result;

        for (;;)
        {
            infoAlloc.descriptorPool =
                m_pools[m_rotateIndex][m_currentPoolIndex[m_rotateIndex]];
            result =
                vkAllocateDescriptorSets(RHIContext::device, &infoAlloc, &set);

            if (result == VK_SUCCESS)
            {
                return RHINativeHandle{set, RHINativeHandleType::DescriptorSet};
            }

            if ((result == VK_ERROR_FRAGMENTED_POOL) ||
                (result == VK_ERROR_OUT_OF_POOL_MEMORY))
            {
                // try next pool
                ++m_currentPoolIndex[m_rotateIndex];
                if (m_currentPoolIndex[m_rotateIndex] >=
                    m_pools[m_rotateIndex].size())
                {
                    createPool();
                }
            }
            else
            {
                WS_ASSERT_VK(result);
                return {};
            }
        }
    }

    VulkanDescriptorAllocator::VulkanDescriptorAllocator()
    {
        m_currentPoolIndex = {0, 0};
    }

    VulkanDescriptorAllocator::~VulkanDescriptorAllocator()
    {
        WS_LOG_DEBUG("Descriptor",
                     "Destroying {} descriptor pools",
                     m_pools[0].size() + m_pools[1].size());
        for (VkDescriptorPool pool : m_pools[0])
        {
            RHIDevice::deletionQueueAdd(
                RHINativeHandle{pool, RHINativeHandleType::DescriptorPool});
        }
        for (VkDescriptorPool pool : m_pools[1])
        {
            RHIDevice::deletionQueueAdd(
                RHINativeHandle{pool, RHINativeHandleType::DescriptorPool});
        }
    }

    void VulkanDescriptorAllocator::resetAll()
    {
        m_rotateIndex = (m_rotateIndex + 1) % m_pools.size();

        for (VkDescriptorPool pool : m_pools[m_rotateIndex])
        {
            vkResetDescriptorPool(RHIContext::device, pool, 0);
        }
        m_currentPoolIndex[m_rotateIndex] = 0;
    }

    RHINativeHandle
    VulkanDescriptorAllocator::allocateSet(RHINativeHandle layout)
    {
        return allocateInternal(layout.asValue<VkDescriptorSetLayout>(),
                                nullptr);
    }

    RHINativeHandle
    VulkanDescriptorAllocator::allocateVariableSet(RHINativeHandle layout,
                                                   std::uint32_t count)
    {
        WS_ASSERT(count <= RHIConfig::MAX_DESCRIPTORS);
        // clang-format off
        VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo = {};
        variableCountInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
        variableCountInfo.descriptorSetCount = 1;
        variableCountInfo.pDescriptorCounts  = &count;
        // clang-format on

        return allocateInternal(layout.asValue<VkDescriptorSetLayout>(),
                                &variableCountInfo);
    }

} // namespace worse