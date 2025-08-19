#include "imgui_impl_vulkan.h"

#include "Math/Rectangle.hpp"
#include "RHIQueue.hpp"
#include "RHIDevice.hpp"
#include "RHISwapchain.hpp"
#include "RHICommandList.hpp"
#include "RHISyncPrimitive.hpp"
#include "RHIBuffer.hpp"
#include "RHITexture.hpp"
#include "Pipeline/RHIPipeline.hpp"
#include "Pipeline/RHIPipelineState.hpp"
#include "Pipeline/RHIRasterizerState.hpp"
#include "Pipeline/RHIDepthStencilState.hpp"

#include <mutex>
#include <unordered_map>

namespace worse
{

    namespace map
    {
        std::mutex mtxImageLayoutMap;
        // cache image transition layouts
        std::unordered_map<u64, RHIImageLayout> imageLayoutMap;

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

    RHICommandList::RHICommandList(RHIQueue* queue, RHINativeHandle cmdPool, std::string_view name)
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
            WS_ASSERT_VK(vkAllocateCommandBuffers(RHIContext::device, &infoAloc, &cmdBuffer));
            m_handle = RHINativeHandle{cmdBuffer, RHINativeHandleType::CommandList};
        }

        m_renderingCompleteBinaySemaphore    = std::make_shared<RHISyncPrimitive>(RHISyncPrimitiveType::BinarySemaphore, "rendering_complete_binary_semaphore");
        m_renderingCompleteTimelineSemaphore = std::make_shared<RHISyncPrimitive>(RHISyncPrimitiveType::TimelineSemaphore, "rendering_complete_timeline_semaphore");
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
        infoBegin.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        WS_ASSERT_VK(vkBeginCommandBuffer(m_handle.asValue<VkCommandBuffer>(), &infoBegin));

        m_state               = RHICommandListState::Recording;
        m_isFirstGraphicsPass = true;
    }

    void RHICommandList::submit(RHISyncPrimitive* semaphoreWait)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        renderPassEnd();
        WS_ASSERT_VK(vkEndCommandBuffer(m_handle.asValue<VkCommandBuffer>()));

        if (!m_renderingCompleteBinaySemaphore->hasBeenWaitedFor)
        {
            m_renderingCompleteBinaySemaphore    = std::make_shared<RHISyncPrimitive>(RHISyncPrimitiveType::BinarySemaphore, m_renderingCompleteBinaySemaphore->getName());
            m_renderingCompleteTimelineSemaphore = std::make_shared<RHISyncPrimitive>(RHISyncPrimitiveType::TimelineSemaphore, m_renderingCompleteTimelineSemaphore->getName());
        }

        m_submissionQueue->submit(m_handle.asValue<VkCommandBuffer>(), 0, semaphoreWait, m_renderingCompleteBinaySemaphore.get(), m_renderingCompleteTimelineSemaphore.get());

        if (semaphoreWait)
        {
            semaphoreWait->setBelongingCmdList(this);
        }

        m_renderingCompleteBinaySemaphore->hasBeenWaitedFor = false;
        m_state                                             = RHICommandListState::Submitted;
    }

    void RHICommandList::renderPassBegin()
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        renderPassEnd();

        if (!(m_pso.type == RHIPipelineType::Graphics))
        {
            return;
        }

        VkRenderingInfo infoRender = {};

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

            texture->convertImageLayout(this, RHIImageLayout::Attachment);

            // clang-format off
            VkRenderingAttachmentInfo colorAttachment = {};
            colorAttachment.sType            = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            colorAttachment.imageView        = texture->getView().asValue<VkImageView>();
            colorAttachment.imageLayout      = vulkanImageLayout(texture->getImageLayout());
            colorAttachment.loadOp           = ((m_pso.rasterizerState->getPolygonMode() == RHIPolygonMode::Wirefame) ||
                                                (m_pso.primitiveTopology == RHIPrimitiveTopology::PointList))
                                                   ? VK_ATTACHMENT_LOAD_OP_LOAD
                                                   : VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.clearValue.color = clearColor;
            // clang-format on

            colorAttachments.push_back(colorAttachment);
        }

        VkRenderingAttachmentInfo depthAttachment = {};
        if (m_pso.renderTargetDepthTexture)
        {
            // clang-format off
            RHITexture* depthTexture = m_pso.renderTargetDepthTexture;

            depthTexture->convertImageLayout(this, RHIImageLayout::Attachment);

            depthAttachment.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depthAttachment.imageView   = depthTexture->getView().asValue<VkImageView>();
            depthAttachment.imageLayout = vulkanImageLayout(depthTexture->getImageLayout());
            depthAttachment.loadOp      = m_pso.clearDepth == 2.0f // means just load
                                             ? VK_ATTACHMENT_LOAD_OP_LOAD
                                             : VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp     = (m_pso.depthStencilState->getDepthWriteEnabled() ||
                                           (m_pso.primitiveTopology == RHIPrimitiveTopology::PointList))
                                             ? VK_ATTACHMENT_STORE_OP_STORE
                                             : VK_ATTACHMENT_STORE_OP_DONT_CARE;

            depthAttachment.clearValue.depthStencil.depth = m_pso.clearDepth;
            // clang-format on

            infoRender.pDepthAttachment = &depthAttachment;
        }

        // clang-format off
        infoRender.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
        infoRender.renderArea           = {m_pso.scissor.x, m_pso.scissor.y, m_pso.scissor.width, m_pso.scissor.height};
        infoRender.layerCount           = 1;
        infoRender.colorAttachmentCount = static_cast<u32>(colorAttachments.size());
        infoRender.pColorAttachments    = colorAttachments.data();
        // clang-format on

        vkCmdBeginRenderingKHR(m_handle.asValue<VkCommandBuffer>(), &infoRender);

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

    void RHICommandList::imguiPassBegin(RHITexture const* renderTarget, math::Rectangle const& scissor)
    {
        // clang-format off
        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView   = renderTarget->getView().asValue<VkImageView>();
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo rendering_info = {};
        rendering_info.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
        rendering_info.renderArea           = {scissor.x, scissor.y, scissor.width, scissor.height};
        rendering_info.layerCount           = 1;
        rendering_info.colorAttachmentCount = 1;
        rendering_info.pColorAttachments    = &colorAttachment;
        rendering_info.pDepthAttachment     = nullptr;

        vkCmdBeginRenderingKHR(m_handle.asValue<VkCommandBuffer>(), &rendering_info);
        // clang-format on
    }
    void RHICommandList::imguiPassEnd(void* drawData)
    {
        ImGui_ImplVulkan_RenderDrawData(static_cast<ImDrawData*>(drawData), m_handle.asValue<VkCommandBuffer>());

        vkCmdEndRenderingKHR(m_handle.asValue<VkCommandBuffer>());
    }

    void RHICommandList::draw(u32 const vertexCount, u32 const vertexOffset)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        vkCmdDraw(m_handle.asValue<VkCommandBuffer>(), vertexCount, 1, vertexOffset, 0);
    }

    void RHICommandList::drawIndexed(u32 const indexCount, u32 const indexOffset, u32 const vertexOffset, u32 const instanceIndex, u32 const instanceCount)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        vkCmdDrawIndexed(m_handle.asValue<VkCommandBuffer>(), indexCount, instanceCount, indexOffset, vertexOffset, instanceIndex);
    }

    void RHICommandList::setPipelineState(RHIPipelineState const& pso)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        WS_ASSERT(pso.isValidated());

        // skip if the pso is not changed
        if (pso.getHash() == m_pso.getHash())
        {
            return;
        }

        m_pso = pso;

        m_pipeline = RHIDevice::getPipeline(pso);

        renderPassBegin();

        VkPipelineBindPoint bindPoint = (m_pso.type == RHIPipelineType::Graphics)
                                            ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                            : VK_PIPELINE_BIND_POINT_COMPUTE;

        vkCmdBindPipeline(m_handle.asValue<VkCommandBuffer>(), bindPoint, m_pipeline->getHandle().asValue<VkPipeline>());

        if (m_pso.type == RHIPipelineType::Graphics)
        {
            setScissor(pso.scissor);
        }

        if ((m_pso.type == RHIPipelineType::Graphics) && m_isFirstGraphicsPass)
        {
            bindGlobalSet();
            m_isFirstGraphicsPass = false;
        }
        if (m_pso.type == RHIPipelineType::Compute)
        {
            bindGlobalSet();
        }

        bindSpecificSet();
    }

    void RHICommandList::clearPipelineState()
    {
        m_pso = {};
    }

    void RHICommandList::dispatch(u32 const x, u32 const y, u32 const z)
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

        vkCmdSetViewport(m_handle.asValue<VkCommandBuffer>(), 0, 1, &vkViewport);
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

    void RHICommandList::insertBarrier(
        RHINativeHandle image,
        RHIFormat const format,
        RHIImageLayout const layoutNew,
        RHIPipelineStageFlags const srcStage,
        RHIAccessFlags const srcAccess,
        RHIPipelineStageFlags const dstStage,
        RHIAccessFlags const dstAccess)
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
        imageBarrier.srcStageMask                    = static_cast<VkPipelineStageFlags2>(srcStage);
        imageBarrier.srcAccessMask                   = static_cast<VkAccessFlags2>(srcAccess);
        imageBarrier.dstStageMask                    = static_cast<VkPipelineStageFlags2>(dstStage);
        imageBarrier.dstAccessMask                   = static_cast<VkAccessFlags2>(dstAccess);
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
        // infoDependency.dependencyFlags         = VK_DEPENDENCY_BY_REGION_BIT;
        infoDependency.imageMemoryBarrierCount = 1;
        infoDependency.pImageMemoryBarriers    = &imageBarrier;
        // clang-format on
        vkCmdPipelineBarrier2KHR(m_handle.asValue<VkCommandBuffer>(), &infoDependency);
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
        srcOffset.x          = static_cast<i32>(source->getWidth());
        srcOffset.y          = static_cast<i32>(source->getHeight());
        srcOffset.z          = 1;
        VkOffset3D dstOffset = {};
        dstOffset.x          = static_cast<i32>(destination->getWidth());
        dstOffset.y          = static_cast<i32>(destination->getHeight());
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
        insertBarrier(destination->getCurrentRt(), destination->getFormat(), RHIImageLayout::TransferDestination, RHIPipelineStageFlagBits::TopOfPipe, RHIAccessFlagBits::MemoryRead, RHIPipelineStageFlagBits::Transfer, RHIAccessFlagBits::MemoryWrite);

        RHIFilter filter = (source->getWidth() == destination->getWidth() &&
                            source->getHeight() == destination->getHeight())
                               ? RHIFilter::Nearest
                               : RHIFilter::Linear;

        // clang-format off
        VkOffset3D srcOffset = {};
        srcOffset.x          = static_cast<i32>(source->getWidth());
        srcOffset.y          = static_cast<i32>(source->getHeight());
        srcOffset.z          = 1;
        VkOffset3D dstOffset = {};
        dstOffset.x          = static_cast<i32>(destination->getWidth());
        dstOffset.y          = static_cast<i32>(destination->getHeight());
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
        insertBarrier(destination->getCurrentRt(), destination->getFormat(), RHIImageLayout::PresentSource, RHIPipelineStageFlagBits::Transfer, RHIAccessFlagBits::MemoryWrite, RHIPipelineStageFlagBits::BottomOfPipe, RHIAccessFlagBits::MemoryRead);
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

    void RHICommandList::copy(RHITexture const* source, RHISwapchain const* destination)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        WS_ASSERT((source->getWidth() == destination->getWidth()) &&
                  (source->getHeight() == destination->getHeight()) &&
                  (source->getFormat() == destination->getFormat()));

        RHIImageLayout sourceInitialLayout = source->getImageLayout();

        source->convertImageLayout(this, RHIImageLayout::TransferSource);
        insertBarrier(destination->getCurrentRt(), destination->getFormat(), RHIImageLayout::TransferDestination, RHIPipelineStageFlagBits::TopOfPipe, RHIAccessFlagBits::MemoryRead, RHIPipelineStageFlagBits::Transfer, RHIAccessFlagBits::MemoryWrite);

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
        region.extent                        = {source->getWidth(), source->getHeight(), 1};

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
        insertBarrier(destination->getCurrentRt(), destination->getFormat(), RHIImageLayout::PresentSource, RHIPipelineStageFlagBits::Transfer, RHIAccessFlagBits::MemoryWrite, RHIPipelineStageFlagBits::BottomOfPipe, RHIAccessFlagBits::MemoryRead);
    }

    void RHICommandList::pushConstants(
        std::span<byte, RHIConfig::MAX_PUSH_CONSTANT_SIZE> data)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        WS_ASSERT(m_pipeline);

        VkShaderStageFlags stageFlags = 0;
        if (m_pso.shaders[RHIShaderType::Compute])
        {
            stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
        if (m_pso.shaders[RHIShaderType::Vertex])
        {
            stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (m_pso.shaders[RHIShaderType::Pixel])
        {
            stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        vkCmdPushConstants(m_handle.asValue<VkCommandBuffer>(), m_pipeline->getLayout().asValue<VkPipelineLayout>(), stageFlags, 0, data.size(), data.data());
    }

    void RHICommandList::setBufferVertex(RHIBuffer* buffer)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        VkBuffer vertexBuffer = buffer->getHandle().asValue<VkBuffer>();
        VkDeviceSize offset   = 0;


        vkCmdBindVertexBuffers(m_handle.asValue<VkCommandBuffer>(), 0, 1, &vertexBuffer, &offset);
    }

    void RHICommandList::setBufferIndex(RHIBuffer* buffer)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);

        VkBuffer indexBuffer  = buffer->getHandle().asValue<VkBuffer>();
        VkDeviceSize offset   = 0;
        VkIndexType indexType = (buffer->getStride() == sizeof(u16)) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
        vkCmdBindIndexBuffer(m_handle.asValue<VkCommandBuffer>(), indexBuffer, offset, indexType);
    }

    void RHICommandList::updateBuffer(RHIBuffer* buffer, u32 const offset, u32 const size, void const* data)
    {
        // clang-format off
        bool synchronizeUpdate = true;
        synchronizeUpdate &= (offset % 4 == 0);
        synchronizeUpdate &= (size % 4 == 0);
        synchronizeUpdate &= (size <= RHIConfig::MAX_BUFFER_UPDATE_SIZE);

        if (synchronizeUpdate)
        {
            vkCmdUpdateBuffer(m_handle.asValue<VkCommandBuffer>(), buffer->getHandle().asValue<VkBuffer>(), offset, size, data);
            VkBufferMemoryBarrier2 barrier = {};
            barrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            barrier.srcStageMask        = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            barrier.srcAccessMask       = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            barrier.dstStageMask        = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            barrier.dstAccessMask       = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.buffer              = buffer->getHandle().asValue<VkBuffer>();
            barrier.offset              = offset;
            barrier.size                = size;

            RHIBufferUsageFlags bufferUsage = buffer->getUsage();
            if ((bufferUsage & RHIBufferUsageFlagBits::Vertex) ||
                (bufferUsage & RHIBufferUsageFlagBits::Instance)) { barrier.dstAccessMask |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT; }
            if (bufferUsage & RHIBufferUsageFlagBits::Index)      { barrier.dstAccessMask |= VK_ACCESS_2_INDEX_READ_BIT; }
            if (bufferUsage & RHIBufferUsageFlagBits::Storage)    { barrier.dstAccessMask |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT; }
            if (bufferUsage & RHIBufferUsageFlagBits::Uniform)    { barrier.dstAccessMask |= VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_UNIFORM_READ_BIT; }

            VkDependencyInfo dependencyInfo = {};
            dependencyInfo.sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            dependencyInfo.bufferMemoryBarrierCount = 1;
            dependencyInfo.pBufferMemoryBarriers    = &barrier;

            vkCmdPipelineBarrier2KHR(m_handle.asValue<VkCommandBuffer>(), &dependencyInfo);
        }
        else
        {
            void* mappedData = static_cast<byte*>(buffer->getMappedData()) + offset;
            std::memcpy(mappedData, data, size);
        }
        // clang-format on
    }

    void RHICommandList::bindGlobalSet()
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        WS_ASSERT(m_pipeline);

        VkDescriptorSet vkSet = RHIDevice::getGlobalDescriptorSet().asValue<VkDescriptorSet>();

        VkPipelineBindPoint bindPoint = (m_pso.type == RHIPipelineType::Graphics)
                                            ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                            : VK_PIPELINE_BIND_POINT_COMPUTE;

        vkCmdBindDescriptorSets(
            m_handle.asValue<VkCommandBuffer>(),
            bindPoint,
            m_pipeline->getLayout().asValue<VkPipelineLayout>(),
            0,
            1,
            &vkSet,
            0,
            nullptr);
    }

    void RHICommandList::bindSpecificSet()
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        WS_ASSERT(m_pipeline);

        u64 hash = m_pipeline->getDescriptorHash();

        RHINativeHandle set = RHIDevice::getSpecificDescriptorSet(hash);
        WS_ASSERT(set);

        VkDescriptorSet vkSet = set.asValue<VkDescriptorSet>();

        VkPipelineBindPoint bindPoint = (m_pso.type == RHIPipelineType::Graphics)
                                            ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                            : VK_PIPELINE_BIND_POINT_COMPUTE;

        vkCmdBindDescriptorSets(
            m_handle.asValue<VkCommandBuffer>(),
            bindPoint,
            m_pipeline->getLayout().asValue<VkPipelineLayout>(),
            1,
            1,
            &vkSet,
            0,
            nullptr);
    }

    void RHICommandList::updateSpecificSet(std::span<RHIDescriptorWrite> writes)
    {
        WS_ASSERT(m_state == RHICommandListState::Recording);
        WS_ASSERT(m_pipeline);

        if (writes.empty())
        {
            return; // Early exit if no writes
        }

        u64 hash            = m_pipeline->getDescriptorHash();
        RHINativeHandle set = RHIDevice::getSpecificDescriptorSet(hash);
        WS_ASSERT(set);

        VkDescriptorSet vkSet = set.asValue<VkDescriptorSet>();

        // Pre-allocate all vectors with known sizes
        std::vector<VkWriteDescriptorSet> vkWrites;
        vkWrites.reserve(writes.size());

        std::vector<VkDescriptorBufferInfo> infoBuffers;
        infoBuffers.reserve(writes.size());

        std::vector<VkDescriptorImageInfo> infoImages;
        infoImages.reserve(writes.size());

        for (RHIDescriptorWrite const& desc : writes)
        {
            switch (desc.type)
            {
            case RHIDescriptorType::Texture:
            case RHIDescriptorType::TextureStorage:
            {
                RHITexture* texture = desc.resource.texture;
                texture             = texture ? texture : RHIDevice::getResourceProvider()->getPlaceholderTexture();

                VkDescriptorType vkType = (desc.type == RHIDescriptorType::Texture)
                                              ? VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
                                              : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

                u32 shift = (desc.type == RHIDescriptorType::Texture)
                                ? RHIConfig::HLSL_REGISTER_SHIFT_T
                                : RHIConfig::HLSL_REGISTER_SHIFT_U;

                VkDescriptorImageInfo infoImage = {};
                infoImage.imageView             = texture->getView().asValue<VkImageView>();
                infoImage.imageLayout           = vulkanImageLayout(texture->getImageLayout());
                infoImage.sampler               = VK_NULL_HANDLE;
                infoImages.push_back(infoImage);

                VkWriteDescriptorSet writeSet = {};
                writeSet.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeSet.dstSet               = vkSet;
                writeSet.dstBinding           = shift + desc.reg;
                writeSet.dstArrayElement      = desc.index;
                writeSet.descriptorCount      = 1;
                writeSet.descriptorType       = vkType;
                writeSet.pImageInfo           = &infoImages.back();
                vkWrites.push_back(writeSet);
                break;
            }
            case RHIDescriptorType::UniformBuffer:
            case RHIDescriptorType::StructuredBuffer:
            case RHIDescriptorType::RWStructuredBuffer:
            {
                RHIBuffer* buffer = desc.resource.buffer;
                WS_ASSERT(buffer); // Validate buffer pointer

                VkDescriptorType vkType = (desc.type == RHIDescriptorType::UniformBuffer)
                                              ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
                                              : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

                u32 shift = 0;
                if (desc.type == RHIDescriptorType::UniformBuffer)
                {
                    shift = RHIConfig::HLSL_REGISTER_SHIFT_B;
                }
                else if (desc.type == RHIDescriptorType::StructuredBuffer)
                {
                    shift = RHIConfig::HLSL_REGISTER_SHIFT_T;
                }
                else if (desc.type == RHIDescriptorType::RWStructuredBuffer)
                {
                    shift = RHIConfig::HLSL_REGISTER_SHIFT_U;
                }

                VkDescriptorBufferInfo infoBuffer = {};
                infoBuffer.buffer                 = buffer->getHandle().asValue<VkBuffer>();
                infoBuffer.offset                 = buffer->getOffset();
                infoBuffer.range                  = buffer->getSize();
                infoBuffers.push_back(infoBuffer);

                VkWriteDescriptorSet writeSet = {};
                writeSet.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeSet.dstSet               = vkSet;
                writeSet.dstBinding           = shift + desc.reg;
                writeSet.dstArrayElement      = desc.index;
                writeSet.descriptorCount      = 1;
                writeSet.descriptorType       = vkType;
                writeSet.pBufferInfo          = &infoBuffers.back();
                vkWrites.push_back(writeSet);
                break;
            }
            default:
                WS_ASSERT_MSG(false, "Unsupported descriptor type");
            }
        }

        vkUpdateDescriptorSets(RHIContext::device, static_cast<u32>(vkWrites.size()), vkWrites.data(), 0, nullptr);
    }

    RHIImageLayout RHICommandList::getImageLayout(RHINativeHandle image)
    {
        return map::getImageLayout(image);
    }

} // namespace worse