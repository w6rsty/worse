#include "RHIDevice.hpp"
#include "RHICommandList.hpp"
#include "Descriptor/RHIBuffer.hpp"

namespace worse
{

    void RHIBuffer::nativeCreate(void const* data)
    {
        // prevent duplicate creation
        nativeDestroy();

        if ((m_type == RHIBufferType::Vertex) ||
            (m_type == RHIBufferType::Index) ||
            (m_type == RHIBufferType::Instance))
        {
            // NOTE: not compatible with VkBufferUsage2
            VkBufferUsageFlags bufferUsage =
                ((m_type == RHIBufferType::Vertex) ||
                 (m_type == RHIBufferType::Instance))
                    ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
                    : VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

            if (m_mappable) // mappable directly
            {
                VkMemoryPropertyFlags memoryProperty =
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                m_handle = RHIDevice::memoryBufferCreate(
                    m_size,
                    bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    memoryProperty,
                    data,
                    m_name);
            }
            else // transfer to GPU using stagging buffer
            {
                RHINativeHandle stagingBuffer;
                std::string stagingName = m_name + "_staging";
                stagingBuffer           = RHIDevice::memoryBufferCreate(
                    m_size,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    data,
                    stagingName);

                m_handle = RHIDevice::memoryBufferCreate(
                    m_size,
                    bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    nullptr, // no data
                    m_name);

                if (RHICommandList* cmdList =
                        RHIDevice::cmdImmediateBegin(RHIQueueType::Transfer))
                {
                    VkCommandBuffer vkCmdBUffer =
                        cmdList->getHandle().asValue<VkCommandBuffer>();

                    VkBufferCopy2 copyRegion = {};
                    copyRegion.sType         = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
                    copyRegion.srcOffset     = 0;
                    copyRegion.dstOffset     = 0;
                    copyRegion.size          = m_size;

                    VkCopyBufferInfo2 infoCopy = {};
                    infoCopy.sType       = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
                    infoCopy.srcBuffer   = stagingBuffer.asValue<VkBuffer>();
                    infoCopy.dstBuffer   = m_handle.asValue<VkBuffer>();
                    infoCopy.regionCount = 1;
                    infoCopy.pRegions    = &copyRegion;

                    vkCmdCopyBuffer2KHR(vkCmdBUffer, &infoCopy);
                    RHIDevice::cmdImmediateSubmit(cmdList);
                    RHIDevice::memoryBufferDestroy(stagingBuffer);
                }
                else
                {
                    WS_LOG_ERROR("RHIBuffer", "Failed to transfer buffer data");
                    RHIDevice::memoryBufferDestroy(stagingBuffer);
                    RHIDevice::memoryBufferDestroy(m_handle);
                }
            }
        }
        else if (m_type == RHIBufferType::Storage)
        {
            // TODO: query this from device
            static constexpr std::size_t MIN_ALIGNMENT = 0x10;
            if (m_stride != MIN_ALIGNMENT)
            {
                // recalculate aligned stride and size
                m_stride = static_cast<std::uint32_t>(
                    (m_stride + MIN_ALIGNMENT - 1) & ~(MIN_ALIGNMENT - 1));
                m_size = m_stride * m_elementCount;
            }

            VkBufferUsageFlags bufferUsage =
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT;

            VkMemoryPropertyFlags memoryProperty =
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            if (m_mappable)
            {
                // TODO: should query accessibility for this triple memory
                // properties combination
                memoryProperty |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            }

            m_handle = RHIDevice::memoryBufferCreate(m_size,
                                                     bufferUsage,
                                                     memoryProperty,
                                                     data,
                                                     m_name);
        }
        else if (m_type == RHIBufferType::Constant)
        {
            // TODO: query this from device
            static constexpr std::size_t MIN_ALIGNMENT = 0x10;
            if (m_stride != MIN_ALIGNMENT)
            {
                // recalculate aligned stride and size
                m_stride = static_cast<std::uint32_t>(
                    (m_stride + MIN_ALIGNMENT - 1) & ~(MIN_ALIGNMENT - 1));
                m_size = m_stride * m_elementCount;
            }

            VkBufferUsageFlags bufferUsage =
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            VkMemoryPropertyFlags memoryProperty =
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            m_handle = RHIDevice::memoryBufferCreate(m_size,
                                                     bufferUsage,
                                                     memoryProperty,
                                                     data,
                                                     m_name);
        }

        WS_ASSERT_MSG(m_handle, "Failed to create buffer");

        m_gpuData = m_mappable ? RHIDevice::memoryGetMappedBufferData(m_handle)
                               : nullptr;
    }

    void RHIBuffer::nativeDestroy()
    {
        if (m_handle)
        {
            RHIDevice::deletionQueueAdd(m_handle);
            m_handle = {};
        }
    }

    void RHIBuffer::update(RHICommandList* cmdList, void const* cpuData,
                           std::uint32_t const size)
    {
        if (!cmdList)
        {
            WS_LOG_ERROR("RHIBuffer",
                         "Failed to update buffer, cmdList is null");
            return;
        }
        WS_ASSERT_MSG(m_mappable, "Cannot update unmappable buffer");
        WS_ASSERT_MSG(m_gpuData, "Cannot update buffer, invalid GPU data");
        WS_ASSERT_MSG(m_offset + size <= m_size, "Update buffer out of range");

        if (m_firstUpdate)
        {
            m_firstUpdate = false;
        }
        else
        {
            m_offset += m_stride;
        }

        cmdList->updateBuffer(this,
                              m_offset,
                              size != 0 ? size : m_stride,
                              cpuData);
    }

} // namespace worse