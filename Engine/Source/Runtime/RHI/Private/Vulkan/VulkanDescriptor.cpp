#include "RHIDevice.hpp"
#include "RHIResource.hpp"
#include "Descriptor/RHIBuffer.hpp"
#include "Descriptor/RHITexture.hpp"
#include "Descriptor/RHISampler.hpp"
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

    void VulkanGlobalSet::createLayout()
    {
        // clang-format off
        VkDescriptorSetLayoutBinding layoutBindings[4] = {};

        // frame data
        layoutBindings[0].binding            = RHIConfig::HLSL_REGISTER_SHIFT_B + 0; // b0
        layoutBindings[0].descriptorCount    = 1;
        layoutBindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBindings[0].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        layoutBindings[0].pImmutableSamplers = nullptr;

        // SamplerComparisonState
        layoutBindings[1].binding            = RHIConfig::HLSL_REGISTER_SHIFT_S + 0; // s0
        layoutBindings[1].descriptorCount    = 1;
        layoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLER;
        layoutBindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBindings[1].pImmutableSamplers = nullptr;
        
        // SamplerState
        layoutBindings[2].binding            = RHIConfig::HLSL_REGISTER_SHIFT_S + 1; // s1
        layoutBindings[2].descriptorCount    = 8;
        layoutBindings[2].descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLER;
        layoutBindings[2].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBindings[2].pImmutableSamplers = nullptr;

        layoutBindings[3].binding            = RHIConfig::HLSL_REGISTER_SHIFT_T + 0; // t0
        layoutBindings[3].descriptorCount    = RHIConfig::MAX_DESCRIPTORS;
        layoutBindings[3].descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        layoutBindings[3].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        layoutBindings[3].pImmutableSamplers = nullptr;

        VkDescriptorBindingFlags bindingFlags[4] = {};
        bindingFlags[0] = 0;
        bindingFlags[1] = 0;
        bindingFlags[2] = 0;
        bindingFlags[3] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

        VkDescriptorSetLayoutBindingFlagsCreateInfo infoBindingFlags = {};
        infoBindingFlags.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        infoBindingFlags.bindingCount  = 4;
        infoBindingFlags.pBindingFlags = bindingFlags;

        VkDescriptorSetLayoutCreateInfo infoLayout = {};
        infoLayout.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        infoLayout.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        infoLayout.pNext        = &infoBindingFlags;
        infoLayout.bindingCount = 4;
        infoLayout.pBindings    = layoutBindings;

        VkDescriptorSetLayout vkLayout = VK_NULL_HANDLE;
        WS_ASSERT_VK(vkCreateDescriptorSetLayout(RHIContext::device,
                                                    &infoLayout,
                                                    nullptr,
                                                    &vkLayout));
        m_layout = RHINativeHandle{vkLayout, RHINativeHandleType::DescriptorSetLayout};
        RHIDevice::setResourceName(m_layout, "global_descriptor_set_layout");
        // clang-format on
    }

    void VulkanGlobalSet::createInfos()
    {
        // clang-format off
        RHIBuffer* buffer = RHIDevice::getResourceProvider()->getFrameConstantBuffer();

        m_frameConstantBufferInfo.buffer = buffer->getHandle().asValue<VkBuffer>();
        m_frameConstantBufferInfo.offset = 0;
        m_frameConstantBufferInfo.range  = buffer->getSize();

        EnumArray<RHISamplerType, RHISampler*> samplers = RHIDevice::getResourceProvider()->getSamplers();

        m_samplerInfos[RHISamplerType::CompareDepth].sampler        = samplers[RHISamplerType::CompareDepth]->getHandle().asValue<VkSampler>();
        m_samplerInfos[RHISamplerType::PointClampEdge].sampler      = samplers[RHISamplerType::PointClampEdge]->getHandle().asValue<VkSampler>();
        m_samplerInfos[RHISamplerType::PointClampBorder].sampler    = samplers[RHISamplerType::PointClampBorder]->getHandle().asValue<VkSampler>();
        m_samplerInfos[RHISamplerType::Wrap].sampler                = samplers[RHISamplerType::Wrap]->getHandle().asValue<VkSampler>();
        m_samplerInfos[RHISamplerType::BilinearClampEdge].sampler   = samplers[RHISamplerType::BilinearClampEdge]->getHandle().asValue<VkSampler>();
        m_samplerInfos[RHISamplerType::BilinearClampBorder].sampler = samplers[RHISamplerType::BilinearClampBorder]->getHandle().asValue<VkSampler>();
        m_samplerInfos[RHISamplerType::BilinearWrap].sampler        = samplers[RHISamplerType::BilinearWrap]->getHandle().asValue<VkSampler>();
        m_samplerInfos[RHISamplerType::TrilinearClamp].sampler      = samplers[RHISamplerType::TrilinearClamp]->getHandle().asValue<VkSampler>();
        m_samplerInfos[RHISamplerType::AnisotropicClamp].sampler    = samplers[RHISamplerType::AnisotropicClamp]->getHandle().asValue<VkSampler>();

        VkImageView imageView = RHIDevice::getResourceProvider()->getPlaceholderTexture()->getView().asValue<VkImageView>();

        m_staticWrites[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        m_staticWrites[0].dstBinding      = RHIConfig::HLSL_REGISTER_SHIFT_B + 0; // b0
        m_staticWrites[0].descriptorCount = 1;
        m_staticWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        m_staticWrites[0].pBufferInfo     = &m_frameConstantBufferInfo;

        m_staticWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        m_staticWrites[1].dstBinding      = RHIConfig::HLSL_REGISTER_SHIFT_S + 0; // s0
        m_staticWrites[1].descriptorCount = 1;
        m_staticWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
        m_staticWrites[1].pImageInfo      = &m_samplerInfos[RHISamplerType::CompareDepth];

        m_staticWrites[2].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        m_staticWrites[2].dstBinding      = RHIConfig::HLSL_REGISTER_SHIFT_S + 1; // s1
        m_staticWrites[2].descriptorCount = 8;
        m_staticWrites[2].descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
        m_staticWrites[2].pImageInfo      = &m_samplerInfos[1];
        // clang-format on
    }

    VulkanGlobalSet::VulkanGlobalSet(VulkanDescriptorAllocator* allocator)
    {
        m_allocator = allocator;

        createLayout();
    }

    VulkanGlobalSet::~VulkanGlobalSet()
    {
        RHIDevice::deletionQueueAdd(m_layout);
    }

    void VulkanGlobalSet::writeAll()
    {
        WS_ASSERT(m_allocator);

        if (m_firstUpdate)
        {
            createInfos();
            m_firstUpdate = false;
        }

        m_set = m_allocator->allocateVariableSet(m_layout,
                                                 RHIConfig::MIN_DESCRIPTORS);

        // write static descriptors
        {
            for (VkWriteDescriptorSet& write : m_staticWrites)
            {
                write.dstSet = m_set.asValue<VkDescriptorSet>();
            }

            vkUpdateDescriptorSets(
                RHIContext::device,
                static_cast<std::uint32_t>(m_staticWrites.size()),
                m_staticWrites.data(),
                0,
                nullptr);
        }
    }

    void VulkanGlobalSet::writeBindlessTextures(
        std::span<RHIBindlessDescriptorWrite> updates)
    {
        WS_ASSERT(m_set);

        std::sort(updates.begin(),
                  updates.end(),
                  [](RHIBindlessDescriptorWrite const& a,
                     RHIBindlessDescriptorWrite const& b)
                  {
                      return a.index < b.index;
                  });

        // <startIndex, length>
        std::vector<std::pair<uint32_t, uint32_t>> ranges;
        for (std::size_t i = 0; i < updates.size(); ++i)
        {
            uint32_t rangeStart = updates[i].index;
            uint32_t rangeCount = 1;

            // Check for contiguous indices
            while ((i + 1 < updates.size()) &&
                   (updates[i + 1].index == rangeStart + rangeCount))
            {
                ++rangeCount;
                ++i;
            }

            ranges.emplace_back(rangeStart, rangeCount);
        }

        std::vector<VkWriteDescriptorSet> writes(ranges.size());
        std::vector<VkDescriptorImageInfo> imageInfos(updates.size());

        uint32_t imageInfoIndex = 0;
        for (std::size_t i = 0; i < ranges.size(); ++i)
        {
            // clang-format off
            auto const& range         = ranges[i];
            writes[i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i].dstSet          = m_set.asValue<VkDescriptorSet>();
            writes[i].dstBinding      = RHIConfig::HLSL_REGISTER_SHIFT_T + 0; // t0
            writes[i].dstArrayElement = range.first;
            writes[i].descriptorCount = range.second;
            writes[i].descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            writes[i].pImageInfo      = imageInfos.data() + imageInfoIndex;

            for (uint32_t j = 0; j < range.second; ++j)
            {
                imageInfos[imageInfoIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfos[imageInfoIndex].imageView   = updates[imageInfoIndex].resource.texture->getView().asValue<VkImageView>();
                imageInfos[imageInfoIndex].sampler     = VK_NULL_HANDLE;
                ++imageInfoIndex;
            }
            // clang-format on
        }

        vkUpdateDescriptorSets(RHIContext::device,
                               static_cast<std::uint32_t>(writes.size()),
                               writes.data(),
                               0,
                               nullptr);
    }

} // namespace worse