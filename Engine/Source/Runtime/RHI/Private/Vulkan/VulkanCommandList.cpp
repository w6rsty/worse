#include "Math/Rectangle.hpp"
#include "RHIQueue.hpp"
#include "RHIDevice.hpp"
#include "RHISwapchain.hpp"
#include "RHICommandList.hpp"
#include "RHISyncPrimitive.hpp"
#include "Pipeline/RHIPipeline.hpp"
#include "Pipeline/RHIPipelineState.hpp"
#include "Descriptor/RHIBuffer.hpp"
#include "Descriptor/RHITexture.hpp"

#include <unordered_map>

namespace worse
{
    namespace map
    {
        std::mutex mtxImageLayoutMap;
        std::unordered_map<std::uint64_t, RHIImageLayout> imageLayoutMap;

        void setImageLayout(RHINativeHandle image, RHIImageLayout layout)
        {
            WS_ASSERT(image);
            std::lock_guard lock{mtxImageLayoutMap};
            imageLayoutMap[image.asValue()] = layout;
        }

        RHIImageLayout getImageLayout(RHINativeHandle image)
        {
            WS_ASSERT(image);
            std::lock_guard lock{mtxImageLayoutMap};
            auto it = imageLayoutMap.find(image.asValue());
            if (it == imageLayoutMap.end())
            {
                // if image has not been set, return RHIImageLayout::Max which
                // indicates the image is fist time used as it will be treated
                // as VK_IMAGE_LAYOUT_UNDEFINED
                return RHIImageLayout::Max;
            }

            return it->second;
        }

        void removeImageLayout(RHINativeHandle image)
        {
            WS_ASSERT(image);
            std::lock_guard lock{mtxImageLayoutMap};
            imageLayoutMap.erase(image.asValue());
        }
    } // namespace map

    namespace descriptorSet
    {
        // bind global descriptor set
        void bindGlobal(RHINativeHandle cmdBuffer, RHIPipelineState const& pso,
                        RHINativeHandle pipelineLayout)
        {
            // global descriptor set is always available
            // it is created during device initialization
            // and never destroyed
            {
                VkDescriptorSet vkSet = RHIDevice::getGlobalDescriptorSet()
                                            .asValue<VkDescriptorSet>();

                VkPipelineBindPoint bindPoint =
                    (pso.type == RHIPipelineType::Graphics)
                        ? VK_PIPELINE_BIND_POINT_GRAPHICS
                        : VK_PIPELINE_BIND_POINT_COMPUTE;

                vkCmdBindDescriptorSets(
                    cmdBuffer.asValue<VkCommandBuffer>(),
                    bindPoint,
                    pipelineLayout.asValue<VkPipelineLayout>(),
                    0,
                    1,
                    &vkSet,
                    0,
                    nullptr);
            }
        }

    } // namespace descriptorSet

    RHICommandList::RHICommandList(RHIQueue* queue, RHINativeHandle cmdPool,
                                   std::string_view name)
        : RHIResource(name)

    {
        m_submissionQueue = queue;

        // Allocate a command buffer from pool
        {
            // clang-format off
            VkCommandBufferAllocateInfo infoAloc = {};
            infoAloc.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            infoAloc.commandPool        = cmdPool.asValue<VkCommandPool>();
            infoAloc.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            infoAloc.commandBufferCount = 1;
            // clang-format on

            VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
            WS_ASSERT_VK(vkAllocateCommandBuffers(RHIContext::device,
                                                  &infoAloc,
                                                  &cmdBuffer));
            m_handle =
                RHINativeHandle{cmdBuffer, RHINativeHandleType::CommandList};
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
        WS_ASSERT_VK(vkBeginCommandBuffer(m_handle.asValue<VkCommandBuffer>(),
                                          &infoBegin));

        m_state = RHICommandListState::Recording;
    }

    void RHICommandList::submit(RHISyncPrimitive* semaphoreWait)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        WS_ASSERT_VK(vkEndCommandBuffer(m_handle.asValue<VkCommandBuffer>()));

        if (!m_renderingCompleteBinaySemaphore->hasBeenWaitedFor)
        {
            m_renderingCompleteBinaySemaphore =
                std::make_shared<RHISyncPrimitive>(
                    RHISyncPrimitiveType::BinarySemaphore,
                    m_renderingCompleteBinaySemaphore->getName());
            m_renderingCompleteTimelineSemaphore =
                std::make_shared<RHISyncPrimitive>(
                    RHISyncPrimitiveType::TimelineSemaphore,
                    m_renderingCompleteTimelineSemaphore->getName());
        }

        m_submissionQueue->submit(m_handle.asValue<VkCommandBuffer>(),
                                  0,
                                  semaphoreWait,
                                  m_renderingCompleteBinaySemaphore.get(),
                                  m_renderingCompleteTimelineSemaphore.get());

        if (semaphoreWait)
        {
            semaphoreWait->setBelongingCmdList(this);
        }

        m_renderingCompleteBinaySemaphore->hasBeenWaitedFor = false;
        m_state = RHICommandListState::Submitted;
    }

    void RHICommandList::renderPassBegin()
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        renderPassEnd();

        VkClearColorValue clearColor = {m_pso.clearColor.r,
                                        m_pso.clearColor.g,
                                        m_pso.clearColor.b,
                                        m_pso.clearColor.a};

        std::vector<VkRenderingAttachmentInfo> colorAttachments;
        for (RHITexture* texture : m_pso.renderTargetColorTextures)
        {
            if (!texture)
            {
                break;
            }

            texture->convertImageLayout(this, RHIImageLayout::ColorAttachment);
            insertBarrier(texture->getImage(),
                          texture->getFormat(),
                          RHIImageLayout::ColorAttachment);

            // clang-format off
            VkRenderingAttachmentInfo colorAttachment = {};
            colorAttachment.sType            = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            colorAttachment.imageView        = texture->getView().asValue<VkImageView>();
            colorAttachment.imageLayout      = vulkanImageLayout(texture->getImageLayout());
            colorAttachment.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.clearValue.color = clearColor;
            // clang-format on

            colorAttachments.push_back(colorAttachment);
        }

        // clang-format off
        VkRenderingInfo infoRender = {};
        infoRender.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
        infoRender.renderArea           = {m_pso.scissor.x, m_pso.scissor.y, m_pso.scissor.width, m_pso.scissor.height};
        infoRender.layerCount           = 1;
        infoRender.colorAttachmentCount = static_cast<std::uint32_t>(colorAttachments.size());
        infoRender.pColorAttachments    = colorAttachments.data();
        // clang-format on

        vkCmdBeginRenderingKHR(m_handle.asValue<VkCommandBuffer>(),
                               &infoRender);

        setViewport(m_pso.viewport);

        m_isRenderPassActive = true;
    }

    void RHICommandList::renderPassEnd()
    {
        if (!m_isRenderPassActive)
        {
            return;
        }

        vkCmdEndRenderingKHR(m_handle.asValue<VkCommandBuffer>());

        m_isRenderPassActive = false;
    }

    void RHICommandList::draw(std::uint32_t const vertexCount,
                              std::uint32_t const vertexOffset)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        vkCmdDraw(m_handle.asValue<VkCommandBuffer>(),
                  vertexCount,
                  1, // instance count
                  vertexOffset,
                  0); // first instance
    }

    void RHICommandList::drawIndexed(std::uint32_t const indexCount,
                                     std::uint32_t const indexOffset,
                                     std::uint32_t const vertexOffset,
                                     std::uint32_t const instanceIndex,
                                     std::uint32_t const instanceCount)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        vkCmdDrawIndexed(m_handle.asValue<VkCommandBuffer>(),
                         indexCount,
                         instanceCount,
                         indexOffset,
                         vertexOffset,
                         instanceIndex);
    }

    void RHICommandList::setPipelineState(RHIPipelineState const& pso)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        WS_ASSERT(pso.isValidated());

        if (pso.getHash() == m_pso.getHash())
        {
            return;
        }

        m_pso = pso;

        RHIDevice::getOrCreatePipeline(pso, m_pipeline, m_descriptorSetLayout);

        renderPassBegin();

        vkCmdBindPipeline(m_handle.asValue<VkCommandBuffer>(),
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_pipeline->getHandle().asValue<VkPipeline>());

        setScissor(pso.scissor);

        // bind pipeline specific resources
        {
            setContantBuffer(
                RHIDevice::getResourceProvider()->getFrameConstantBuffer(),
                0);

            descriptorSet::bindGlobal(m_handle, pso, m_pipeline->getLayout());
        }
    }

    void RHICommandList::clearPipelineState()
    {
        m_pso = {};
    }

    void RHICommandList::dipatch(std::uint32_t const x, std::uint32_t const y,
                                 std::uint32_t const z)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        vkCmdDispatch(m_handle.asValue<VkCommandBuffer>(), x, y, z);
    }

    void RHICommandList::setViewport(RHIViewport const& viewport)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        VkViewport vkViewport = {};
        vkViewport.x          = viewport.x;
        vkViewport.y          = viewport.y;
        vkViewport.width      = viewport.width;
        vkViewport.height     = viewport.height;
        vkViewport.minDepth   = viewport.depthMin;
        vkViewport.maxDepth   = viewport.depthMax;

        vkCmdSetViewport(m_handle.asValue<VkCommandBuffer>(),
                         0,
                         1,
                         &vkViewport);
    }

    void RHICommandList::setScissor(math::Rectangle const& scissor)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        VkRect2D vkScissor      = {};
        vkScissor.offset.x      = scissor.x;
        vkScissor.offset.y      = scissor.y;
        vkScissor.extent.width  = scissor.width;
        vkScissor.extent.height = scissor.height;

        vkCmdSetScissor(m_handle.asValue<VkCommandBuffer>(), 0, 1, &vkScissor);
    }

    void RHICommandList::insertBarrier(RHINativeHandle image,
                                       RHIFormat const format,
                                       RHIImageLayout const layoutNew)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        WS_ASSERT(image);
        RHIImageLayout currentLayout = getImageLayout(image);
        if (currentLayout == layoutNew)
        {
            return;
        }

        // clang-format off
        VkImageMemoryBarrier2 imageBarrier = {};
        imageBarrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        imageBarrier.srcStageMask                    = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        imageBarrier.srcAccessMask                   = VK_ACCESS_2_MEMORY_WRITE_BIT;
        imageBarrier.dstStageMask                    = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        imageBarrier.dstAccessMask                   = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
        imageBarrier.oldLayout                       = vulkanImageLayout(currentLayout);
        imageBarrier.newLayout                       = vulkanImageLayout(layoutNew);
        imageBarrier.image                           = image.asValue<VkImage>();
        imageBarrier.subresourceRange.aspectMask     = vulkanImageAspectFlags(format);
        imageBarrier.subresourceRange.baseMipLevel   = 0;
        imageBarrier.subresourceRange.levelCount     = 1;
        imageBarrier.subresourceRange.baseArrayLayer = 0;
        imageBarrier.subresourceRange.layerCount     = 1;
        // clang-format on

        // clang-format off
        VkDependencyInfo infoDependency        = {};
        infoDependency.sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        infoDependency.imageMemoryBarrierCount = 1;
        infoDependency.pImageMemoryBarriers    = &imageBarrier;
        // clang-format on
        vkCmdPipelineBarrier2KHR(m_handle.asValue<VkCommandBuffer>(),
                                 &infoDependency);
        map::setImageLayout(image, layoutNew);
    }

    void RHICommandList::blit(RHITexture const* source,
                              RHITexture const* destination)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        WS_ASSERT(source != destination);

        // clang-format off
        RHIImageLayout sourceInitialLayout      = source->getImageLayout();
        RHIImageLayout destinationInitialLayout = destination->getImageLayout();

        source->convertImageLayout(this, RHIImageLayout::TransferSource);
        destination->convertImageLayout(this, RHIImageLayout::TransferDestination);

        RHIFilter filter = (source->getWidth() == destination->getWidth() &&
                            source->getHeight() == destination->getHeight())
                               ? RHIFilter::Nearest
                               : RHIFilter::Linear;

        VkOffset3D srcOffset = {};
        srcOffset.x          = static_cast<std::int32_t>(source->getWidth());
        srcOffset.y          = static_cast<std::int32_t>(source->getHeight());
        srcOffset.z          = 1;
        VkOffset3D dstOffset = {};
        dstOffset.x          = static_cast<std::int32_t>(destination->getWidth());
        dstOffset.y          = static_cast<std::int32_t>(destination->getHeight());
        dstOffset.z          = 1;

        VkImageBlit2 region = {};
        region.sType                         = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
        region.srcOffsets[0]                 = {0, 0, 0};
        region.srcOffsets[1]                 = srcOffset;
        region.srcSubresource.mipLevel       = 0;
        region.srcSubresource.layerCount     = 1;
        region.srcSubresource.baseArrayLayer = 0;
        region.srcSubresource.aspectMask     = vulkanImageAspectFlags(source->getFormat());
        region.dstOffsets[0]                 = {0, 0, 0};
        region.dstOffsets[1]                 = dstOffset;
        region.dstSubresource.mipLevel       = 0;
        region.dstSubresource.layerCount     = 1;
        region.dstSubresource.baseArrayLayer = 0;
        region.dstSubresource.aspectMask     = vulkanImageAspectFlags(destination->getFormat());

        VkBlitImageInfo2 info = {};
        info.sType          = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
        info.srcImage       = source->getImage().asValue<VkImage>();
        info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        info.dstImage       = destination->getImage().asValue<VkImage>();
        info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        info.regionCount    = 1;
        info.pRegions       = &region;
        info.filter         = filter == RHIFilter::Nearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
        // clang-format on

        vkCmdBlitImage2KHR(m_handle.asValue<VkCommandBuffer>(), &info);

        // restore layout
        source->convertImageLayout(this, sourceInitialLayout);
        destination->convertImageLayout(this, destinationInitialLayout);
    }

    void RHICommandList::blit(RHITexture const* source,
                              RHISwapchain const* destination)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        RHIImageLayout initialLayout = source->getImageLayout();

        source->convertImageLayout(this, RHIImageLayout::TransferSource);
        insertBarrier(destination->getCurrentRt(),
                      destination->getFormat(),
                      RHIImageLayout::TransferDestination);

        RHIFilter filter = (source->getWidth() == destination->getWidth() &&
                            source->getHeight() == destination->getHeight())
                               ? RHIFilter::Nearest
                               : RHIFilter::Linear;

        // clang-format off
        VkOffset3D srcOffset = {};
        srcOffset.x          = static_cast<std::int32_t>(source->getWidth());
        srcOffset.y          = static_cast<std::int32_t>(source->getHeight());
        srcOffset.z          = 1;
        VkOffset3D dstOffset = {};
        dstOffset.x          = static_cast<std::int32_t>(destination->getWidth());
        dstOffset.y          = static_cast<std::int32_t>(destination->getHeight());
        dstOffset.z          = 1;
        
        VkImageBlit2 region  = {};
        region.sType                         = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
        region.srcOffsets[0]                 = {0, 0, 0};
        region.srcOffsets[1]                 = srcOffset;
        region.srcSubresource.mipLevel       = 0;
        region.srcSubresource.layerCount     = 1;
        region.srcSubresource.baseArrayLayer = 0;
        region.srcSubresource.aspectMask     = vulkanImageAspectFlags(source->getFormat());
        region.dstOffsets[0]                 = {0, 0, 0};
        region.dstOffsets[1]                 = dstOffset;
        region.dstSubresource.mipLevel       = 0;
        region.dstSubresource.layerCount     = 1;
        region.dstSubresource.baseArrayLayer = 0;
        region.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;

        VkBlitImageInfo2 info = {};
        info.sType          = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
        info.srcImage       = source->getImage().asValue<VkImage>();
        info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        info.dstImage       = destination->getCurrentRt().asValue<VkImage>();
        info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        info.regionCount    = 1;
        info.pRegions       = &region;
        info.filter         = filter == RHIFilter::Nearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
        // clang-format on

        vkCmdBlitImage2KHR(m_handle.asValue<VkCommandBuffer>(), &info);

        // restore layout
        source->convertImageLayout(this, initialLayout);
        insertBarrier(destination->getCurrentRt(),
                      destination->getFormat(),
                      RHIImageLayout::PresentSource);
    }

    void RHICommandList::copy(RHITexture const* source,
                              RHITexture const* destination)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        WS_ASSERT((source->getWidth() == destination->getWidth()) &&
                  (source->getHeight() == destination->getHeight()) &&
                  (source->getFormat() == destination->getFormat()));

        RHIImageLayout sourceInitialLayout      = source->getImageLayout();
        RHIImageLayout destinationInitialLayout = destination->getImageLayout();

        // clang-format off
        source->convertImageLayout(this, RHIImageLayout::TransferSource);
        destination->convertImageLayout(this, RHIImageLayout::TransferDestination);

        VkImageCopy2 region = {};
        region.sType                         = VK_STRUCTURE_TYPE_IMAGE_COPY_2;
        region.srcOffset                     = {0, 0, 0};
        region.srcSubresource.mipLevel       = 0;
        region.srcSubresource.layerCount     = 1;
        region.srcSubresource.baseArrayLayer = 0;
        region.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.dstOffset                     = {0, 0, 0};
        region.dstSubresource.mipLevel       = 0;
        region.dstSubresource.layerCount     = 1;
        region.dstSubresource.baseArrayLayer = 0;
        region.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.extent                        = {source->getWidth(), source->getHeight(), 1};

        VkCopyImageInfo2 info = {};
        info.sType          = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2;
        info.srcImage       = source->getImage().asValue<VkImage>();
        info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        info.dstImage       = destination->getImage().asValue<VkImage>();
        info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        info.regionCount    = 1;
        info.pRegions       = &region;
        // clang-format on

        vkCmdCopyImage2KHR(m_handle.asValue<VkCommandBuffer>(), &info);

        // restore layout
        source->convertImageLayout(this, sourceInitialLayout);
        destination->convertImageLayout(this, destinationInitialLayout);
    }

    void RHICommandList::copy(RHITexture const* source,
                              RHISwapchain const* destination)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        WS_ASSERT((source->getWidth() == destination->getWidth()) &&
                  (source->getHeight() == destination->getHeight()) &&
                  (source->getFormat() == destination->getFormat()));

        RHIImageLayout sourceInitialLayout = source->getImageLayout();

        source->convertImageLayout(this, RHIImageLayout::TransferSource);
        insertBarrier(destination->getCurrentRt(),
                      destination->getFormat(),
                      RHIImageLayout::TransferDestination);

        VkImageCopy2 region                  = {};
        region.sType                         = VK_STRUCTURE_TYPE_IMAGE_COPY_2;
        region.srcOffset                     = {0, 0, 0};
        region.srcSubresource.mipLevel       = 0;
        region.srcSubresource.layerCount     = 1;
        region.srcSubresource.baseArrayLayer = 0;
        region.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.dstOffset                     = {0, 0, 0};
        region.dstSubresource.mipLevel       = 0;
        region.dstSubresource.layerCount     = 1;
        region.dstSubresource.baseArrayLayer = 0;
        region.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.extent = {source->getWidth(), source->getHeight(), 1};

        VkCopyImageInfo2 info = {};
        info.sType            = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2;
        info.srcImage         = source->getImage().asValue<VkImage>();
        info.srcImageLayout   = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        info.dstImage         = destination->getCurrentRt().asValue<VkImage>();
        info.dstImageLayout   = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        info.regionCount      = 1;
        info.pRegions         = &region;

        vkCmdCopyImage2KHR(m_handle.asValue<VkCommandBuffer>(), &info);

        // restore
        source->convertImageLayout(this, sourceInitialLayout);
        insertBarrier(destination->getCurrentRt(),
                      destination->getFormat(),
                      RHIImageLayout::PresentSource);
    }

    void RHICommandList::setBufferVertex(RHIBuffer* buffer)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        VkBuffer vertexBuffer = buffer->getHandle().asValue<VkBuffer>();
        VkDeviceSize offset   = 0;

        vkCmdBindVertexBuffers(m_handle.asValue<VkCommandBuffer>(),
                               0,
                               1,
                               &vertexBuffer,
                               &offset);
    }

    void RHICommandList::setBufferIndex(RHIBuffer* buffer)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        VkBuffer indexBuffer  = buffer->getHandle().asValue<VkBuffer>();
        VkDeviceSize offset   = 0;
        VkIndexType indexType = (buffer->getStride() == sizeof(std::uint16_t))
                                    ? VK_INDEX_TYPE_UINT16
                                    : VK_INDEX_TYPE_UINT32;
        vkCmdBindIndexBuffer(m_handle.asValue<VkCommandBuffer>(),
                             indexBuffer,
                             offset,
                             indexType);
    }

    void RHICommandList::setContantBuffer(RHIBuffer* buffer,
                                          std::uint32_t const slot)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        if (!m_descriptorSetLayout)
        {
            WS_LOG_WARN("RHICommandList",
                        "Cannot set constant buffer out of render pass");
            return;
        }

        m_descriptorSetLayout->setConstantBuffer(buffer, slot);
    }

    void RHICommandList::setBuffer(RHIBuffer* buffer, std::uint32_t const slot)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        if (!m_descriptorSetLayout)
        {
            WS_LOG_WARN("RHICommandList",
                        "Cannot set buffer out of render pass");
            return;
        }

        m_descriptorSetLayout->setBuffer(buffer, slot);
    }

    void RHICommandList::updateBuffer(RHIBuffer* buffer,
                                      std::uint32_t const offset,
                                      std::uint32_t const size,
                                      void const* data)
    {
        bool synchronizeUpdate = true;
        synchronizeUpdate &= (offset % 4 == 0);
        synchronizeUpdate &= (size % 4 == 0);
        synchronizeUpdate &= (size <= RHIConfig::MAX_BUFFER_UPDATE_SIZE);

        if (synchronizeUpdate)
        {
            vkCmdUpdateBuffer(m_handle.asValue<VkCommandBuffer>(),
                              buffer->getHandle().asValue<VkBuffer>(),
                              offset,
                              size,
                              data);
            VkBufferMemoryBarrier2 barrier = {};
            barrier.sType         = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            barrier.srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            barrier.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.buffer = buffer->getHandle().asValue<VkBuffer>();
            barrier.offset = offset;
            barrier.size   = size;

            switch (buffer->getType())
            {
            case RHIBufferType::Vertex:
            case RHIBufferType::Instance:
                barrier.dstAccessMask |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
                break;
            case RHIBufferType::Index:
                barrier.dstAccessMask |= VK_ACCESS_2_INDEX_READ_BIT;
                break;
            case RHIBufferType::Storage:
                barrier.dstAccessMask |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
                break;
            case RHIBufferType::Constant:
                barrier.dstAccessMask |=
                    VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_UNIFORM_READ_BIT;
                break;
            default:
                WS_LOG_ERROR("RHICommandList", "Unknown buffer type");
                break;
            }

            VkDependencyInfo dependencyInfo = {};
            dependencyInfo.sType            = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            dependencyInfo.bufferMemoryBarrierCount = 1;
            dependencyInfo.pBufferMemoryBarriers    = &barrier;

            vkCmdPipelineBarrier2KHR(m_handle.asValue<VkCommandBuffer>(),
                                     &dependencyInfo);
        }
        else
        {
            void* mappedData =
                static_cast<std::byte*>(buffer->getMappedData()) + offset;
            std::memcpy(mappedData, data, size);
        }
    }

    RHIImageLayout RHICommandList::getImageLayout(RHINativeHandle image)
    {
        return map::getImageLayout(image);
    }

} // namespace worse