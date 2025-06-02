#include "RHICommandList.hpp"
#include "RHIQueue.hpp"
#include "RHISyncPrimitive.hpp"

namespace worse
{

    RHICommandList::RHICommandList(RHIQueue* queue, RHIResource cmdPool,
                                   char const* name)
    {
        m_queue = queue;

        // Allocate a command buffer from pool
        {
            VkCommandBufferAllocateInfo infoAloc = {};
            infoAloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            infoAloc.commandPool        = cmdPool.asValue<VkCommandPool>();
            infoAloc.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            infoAloc.commandBufferCount = 1;

            VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
            WS_ASSERT_VK(vkAllocateCommandBuffers(RHIContext::device,
                                                  &infoAloc,
                                                  &cmdBuffer));
            m_cmdList = RHIResource{cmdBuffer, RHIResourceType::CommandList};
        }

        m_renderingCompleteBinaySemaphore = std::make_shared<RHISyncPrimitive>(
            RHISyncPrimitiveType::BinarySemaphore,
            "rendering_complete_binary_semaphore");
        m_renderingCompleteTimelineSemaphore =
            std::make_shared<RHISyncPrimitive>(
                RHISyncPrimitiveType::TimelineSemaphore,
                "rendering_complete_timeline_semaphore");
    }

    RHICommandList::~RHICommandList()
    {
        // Reset shared_ptrs to ensure proper cleanup order
        m_renderingCompleteBinaySemaphore.reset();
        m_renderingCompleteTimelineSemaphore.reset();
    }

    void RHICommandList::begin()
    {
        WS_ASSERT(m_state == RHICommandListState::Idle);

        VkCommandBufferBeginInfo infoBegin = {};
        infoBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        WS_ASSERT_VK(vkBeginCommandBuffer(m_cmdList.asValue<VkCommandBuffer>(),
                                          &infoBegin));

        m_state = RHICommandListState::Recording;
    }

    void RHICommandList::submit(RHISyncPrimitive* semaphoreWait)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        WS_ASSERT_VK(vkEndCommandBuffer(m_cmdList.asValue<VkCommandBuffer>()));

        if (!m_renderingCompleteBinaySemaphore->hasBeenWaitedFor)
        {
            m_renderingCompleteBinaySemaphore =
                std::make_shared<RHISyncPrimitive>(
                    RHISyncPrimitiveType::BinarySemaphore,
                    "rendering_complete_binary_semaphore");
            m_renderingCompleteTimelineSemaphore =
                std::make_shared<RHISyncPrimitive>(
                    RHISyncPrimitiveType::TimelineSemaphore,
                    "rendering_complete_timeline_semaphore");
        }

        m_queue->submit(m_cmdList.asValue<VkCommandBuffer>(),
                        0,
                        semaphoreWait,
                        m_renderingCompleteBinaySemaphore.get(),
                        m_renderingCompleteTimelineSemaphore.get());

        if (semaphoreWait)
        {
            semaphoreWait->setCmdList(this);
        }

        m_renderingCompleteBinaySemaphore->hasBeenWaitedFor = false;
        m_state = RHICommandListState::Submitted;
    }

    void RHICommandList::insertBarrier(RHIResource image,
                                       RHIFormat const format,
                                       std::uint32_t const mipIndex,
                                       std::uint32_t const mipRange,
                                       std::uint32_t const arrayLength,
                                       RHIImageLayout const layoutNew)
    {
        VkImageMemoryBarrier2KHR imageBarrier = {};
        imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR;
        imageBarrier.srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR;
        imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR;
        imageBarrier.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR;
        imageBarrier.dstAccessMask =
            VK_ACCESS_2_MEMORY_READ_BIT_KHR | VK_ACCESS_2_MEMORY_WRITE_BIT_KHR;
        imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        ;
        imageBarrier.image                         = image.asValue<VkImage>();
        imageBarrier.subresourceRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBarrier.subresourceRange.baseMipLevel = mipIndex;
        imageBarrier.subresourceRange.levelCount   = mipRange;
        imageBarrier.subresourceRange.baseArrayLayer = 0;
        imageBarrier.subresourceRange.layerCount     = arrayLength;

        VkDependencyInfo infoDependency = {};
        infoDependency.sType            = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        infoDependency.imageMemoryBarrierCount = 1;
        infoDependency.pImageMemoryBarriers    = &imageBarrier;

        vkCmdPipelineBarrier2KHR(m_cmdList.asValue<VkCommandBuffer>(),
                                 &infoDependency);
    }

} // namespace worse