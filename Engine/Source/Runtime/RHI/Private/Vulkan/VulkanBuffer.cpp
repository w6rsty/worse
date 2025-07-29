#include "RHIDevice.hpp"
#include "RHICommandList.hpp"
#include "RHIBuffer.hpp"

#include <bit>

namespace worse
{

    void RHIBuffer::nativeCreate(void const* data)
    {
        // prevent duplicate creation
        nativeDestroy();

        bool isVIIOnly     = false;
        bool isStorageOnly = false;
        bool isVIIStorage  = false;
        bool isUniform     = false;

        // validation
        {
            WS_ASSERT(m_usage != RHIBufferUsageFlagBits::None);

            if (m_usage & RHIBufferUsageFlagBits::Uniform)
            {
                WS_ASSERT((m_usage & RHIBufferUsageFlagBits::Uniform) == RHIBufferUsageFlagBits::Uniform);

                isUniform = true;

                if (m_mappable)
                {
                    WS_ASSERT_MSG(data != nullptr,
                                  "Uniform buffer must have data if mappable");
                }
            }

            // contain vertex/index/instance
            if (m_usage & VII_BIT)
            {
                WS_ASSERT_MSG(std::popcount(static_cast<u8>(m_usage & VII_MASK)) == 1,
                              "RHIBuffer usage must specify exactly one of: Vertex, Index, or Instance");

                // vertex/index/instance only (check if no storage flag)
                if ((m_usage & RHIBufferUsageFlagBits::Storage) == 0)
                {
                    WS_ASSERT_MSG(data != nullptr,
                                  "Vertex/Index/Instance buffer must have data");
                    isVIIOnly = true;
                }
                else
                {
                    isVIIStorage = true;
                }
            }

            // storage only data is optional
            isStorageOnly = (m_usage & RHIBufferUsageFlagBits::Storage) == RHIBufferUsageFlagBits::Storage;
        }

        if (isStorageOnly || isUniform)
        {
            // correct alignment
            // TODO: query this from device
            static constexpr usize MIN_ALIGNMENT = 0x10;
            if (m_stride != MIN_ALIGNMENT)
            {
                m_stride = static_cast<u32>((m_stride + MIN_ALIGNMENT - 1) & ~(MIN_ALIGNMENT - 1));
                m_size   = m_stride * m_elementCount;
            }
        }

        VkBufferUsageFlags bufferUsage       = 0;
        VkMemoryPropertyFlags memoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        // buffer usage and memory properties
        {

            if ((m_usage & RHIBufferUsageFlagBits::Vertex) || m_usage & RHIBufferUsageFlagBits::Instance)
            {
                bufferUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            }
            if (m_usage & RHIBufferUsageFlagBits::Index)
            {
                bufferUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            }
            if (m_usage & RHIBufferUsageFlagBits::Storage)
            {
                bufferUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            }
            if (m_usage & RHIBufferUsageFlagBits::Uniform)
            {
                bufferUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }

            // vertex/index/instance only
            if (isVIIOnly)
            {
                bufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
            // storage only
            if (isStorageOnly)
            {
                bufferUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
            if (isUniform)
            {
                memoryProperty |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            }
        }

        bool isStaggingCreation = isVIIOnly || (isStorageOnly && data);
        bool isDirectlyCreation = isVIIStorage || (isStorageOnly && !data);

        if (isStaggingCreation)
        {
            RHINativeHandle stagingBuffer = RHIDevice::memoryBufferCreate(
                m_size,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                data,
                {m_name + "_staging"});

            m_handle = RHIDevice::memoryBufferCreate(
                m_size,
                bufferUsage,
                memoryProperty,
                nullptr,
                m_name);

            if (RHICommandList* cmdList = RHIDevice::cmdImmediateBegin(RHIQueueType::Transfer))
            {
                VkCommandBuffer vkCmdBUffer = cmdList->getHandle().asValue<VkCommandBuffer>();

                VkBufferCopy2 copyRegion = {};
                copyRegion.sType         = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
                copyRegion.srcOffset     = 0;
                copyRegion.dstOffset     = 0;
                copyRegion.size          = m_size;

                VkCopyBufferInfo2 infoCopy = {};
                infoCopy.sType             = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
                infoCopy.srcBuffer         = stagingBuffer.asValue<VkBuffer>();
                infoCopy.dstBuffer         = m_handle.asValue<VkBuffer>();
                infoCopy.regionCount       = 1;
                infoCopy.pRegions          = &copyRegion;

                vkCmdCopyBuffer2KHR(vkCmdBUffer, &infoCopy);
                RHIDevice::cmdImmediateSubmit(cmdList);
                RHIDevice::memoryBufferDestroy(stagingBuffer);
            }
            else // error handling
            {
                WS_LOG_ERROR("RHIBuffer", "Failed to transfer buffer data");
                RHIDevice::memoryBufferDestroy(stagingBuffer);
                RHIDevice::memoryBufferDestroy(m_handle);
            }
        }
        else if (isDirectlyCreation || isUniform)
        {
            m_handle = RHIDevice::memoryBufferCreate(
                m_size,
                bufferUsage,
                memoryProperty,
                data,
                m_name);
        }

        WS_ASSERT_MSG(m_handle, "Failed to create buffer");

        m_gpuData = (isUniform && m_mappable) ? RHIDevice::memoryGetMappedBufferData(m_handle) : nullptr;
    }

    void RHIBuffer::nativeDestroy()
    {
        if (m_handle)
        {
            RHIDevice::deletionQueueAdd(m_handle);
            m_handle = {};
        }
    }

    void RHIBuffer::update(RHICommandList* cmdList, void const* cpuData, u32 const size)
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

        cmdList->updateBuffer(this, m_offset, size != 0 ? size : m_stride, cpuData);
    }

} // namespace worse