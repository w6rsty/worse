#include "RHIBuffer.hpp"
#include "RHITexture.hpp"
#include "RHIDescriptor.hpp"
#include "RHIDescriptorSet.hpp"

#include <vector>

namespace worse
{

    void RHIDescriptorSet::update(std::vector<RHIDescriptor> const& descriptors)
    {
        m_descriptors = descriptors;

        WS_ASSERT(m_handle);

        std::vector<VkWriteDescriptorSet> writeDescriptorSets;
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        std::vector<VkDescriptorImageInfo> imageInfos;

        writeDescriptorSets.reserve(descriptors.size());
        bufferInfos.reserve(descriptors.size());
        imageInfos.reserve(descriptors.size());

        for (RHIDescriptor const& descriptor : descriptors)
        {
            // Skip
            if (descriptor.type == RHIDescriptorType::PushConstantBuffer)
            {
                continue;
            }

            // clang-format off
            VkWriteDescriptorSet writeSet = {};
            writeSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeSet.dstSet          = m_handle.asValue<VkDescriptorSet>();
            writeSet.dstBinding      = descriptor.slot;
            writeSet.dstArrayElement = 0;
            writeSet.descriptorCount = descriptor.isArray ? descriptor.arrayLength : 1;
            writeSet.descriptorType  = vulkanDescriptorType(descriptor.type);

            if ((descriptor.type == RHIDescriptorType::Image) ||
                (descriptor.type == RHIDescriptorType::TextureStorage))
            {
                // For storage images (UAV textures)
                WS_ASSERT(descriptor.data.texture);

                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageView   = descriptor.data.texture->getRtv().asValue<VkImageView>();
                imageInfo.imageLayout = vulkanImageLayout(descriptor.layout);
                imageInfo.sampler     = VK_NULL_HANDLE;

                imageInfos.push_back(imageInfo);
                writeSet.pImageInfo = &imageInfos.back();
            }
            else if ((descriptor.type == RHIDescriptorType::ConstantBuffer) ||
                     (descriptor.type == RHIDescriptorType::StructuredBuffer))
            {
                // For storage buffers (structured buffers)
                WS_ASSERT(descriptor.data.buffer);

                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = descriptor.data.buffer->getHandle().asValue<VkBuffer>();
                bufferInfo.offset = descriptor.dynamicOffset;
                bufferInfo.range  = descriptor.range > 0
                                        ? descriptor.range
                                        : descriptor.data.buffer->getSize();

                bufferInfos.push_back(bufferInfo);
                writeSet.pBufferInfo = &bufferInfos.back();
            }

            writeDescriptorSets.push_back(writeSet);
        }
        // clang-format on

        if (!writeDescriptorSets.empty())
        {
            vkUpdateDescriptorSets(
                RHIContext::device,
                static_cast<std::uint32_t>(writeDescriptorSets.size()),
                writeDescriptorSets.data(),
                0,
                nullptr);
        }
    }

} // namespace worse