#include "RHIDevice.hpp"
#include "RHICommandList.hpp"
#include "RHITexture.hpp"

namespace worse
{
    namespace
    {
        void uploadTexture(RHITexture& texture)
        {
            // clang-format off
            usize size = texture.getMip(0, 0).bytes.size();

            RHINativeHandle stagingBuffer = RHIDevice::memoryBufferCreate(
                static_cast<u32>(size),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                texture.getMip(0, 0).bytes.data(),
                "texture_staging_buffer");
            WS_ASSERT(stagingBuffer);

            if (RHICommandList* cmdList =
                    RHIDevice::cmdImmediateBegin(RHIQueueType::Graphics))
            {
                cmdList->insertBarrier(texture.getImage(),
                                       texture.getFormat(),
                                       RHIImageLayout::TransferDestination);

                VkBufferImageCopy2 copyRegion = {};
                copyRegion.sType                           = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
                copyRegion.bufferOffset                    = 0;
                copyRegion.bufferRowLength                 = 0;
                copyRegion.bufferImageHeight               = 0;
                copyRegion.imageSubresource.aspectMask     = vulkanImageAspectFlags(texture.getFormat());
                copyRegion.imageSubresource.baseArrayLayer = 0;
                copyRegion.imageSubresource.layerCount     = 1;
                copyRegion.imageSubresource.mipLevel       = 0;
                copyRegion.imageOffset                     = {0, 0, 0};
                copyRegion.imageExtent                     = {texture.getWidth(),
                                                              texture.getHeight(),
                                                              texture.getDepth()};

                VkCopyBufferToImageInfo2 infoCopy = {};
                infoCopy.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2;
                infoCopy.srcBuffer      = stagingBuffer.asValue<VkBuffer>();
                infoCopy.dstImage       = texture.getImage().asValue<VkImage>();
                infoCopy.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                infoCopy.regionCount    = 1;
                infoCopy.pRegions       = &copyRegion;

                vkCmdCopyBufferToImage2KHR(cmdList->getHandle().asValue<VkCommandBuffer>(), &infoCopy);

                RHIDevice::cmdImmediateSubmit(cmdList);

                RHIDevice::memoryBufferDestroy(stagingBuffer);
            }
            // clang-format on
        }

        void createImageView(RHINativeHandle image, RHINativeHandle& imageView,
                             RHITexture* texture)
        {
            VkImageViewType viewType = vulkanImageViewType(texture->getType());
            WS_ASSERT(viewType != VK_IMAGE_VIEW_TYPE_MAX_ENUM);

            VkImageViewCreateInfo infoImageView = {};
            infoImageView.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            infoImageView.image    = image.asValue<VkImage>();
            infoImageView.viewType = viewType;
            infoImageView.format   = vulkanFormat(texture->getFormat());
            infoImageView.subresourceRange.aspectMask =
                vulkanImageAspectFlags(texture->getFormat());
            infoImageView.subresourceRange.baseMipLevel = 0;
            infoImageView.subresourceRange.levelCount   = 1;

            if (texture->getType() == RHITextureType::TextureCube)
            {
                infoImageView.subresourceRange.baseArrayLayer = 0;
                infoImageView.subresourceRange.layerCount     = 6;
            }
            else if (texture->getType() == RHITextureType::Texture3D)
            {
                infoImageView.subresourceRange.baseArrayLayer = 0;
                infoImageView.subresourceRange.layerCount     = 1;
            }
            else
            {
                infoImageView.subresourceRange.baseArrayLayer = 0;
                infoImageView.subresourceRange.layerCount     = 1;
            }

            infoImageView.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            infoImageView.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            infoImageView.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            infoImageView.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            VkImageView vkImageView = VK_NULL_HANDLE;
            WS_ASSERT_VK(vkCreateImageView(RHIContext::device,
                                           &infoImageView,
                                           nullptr,
                                           &vkImageView));
            imageView =
                RHINativeHandle{vkImageView, RHINativeHandleType::ImageView};
            RHIDevice::setResourceName(imageView, texture->getName());
        }
    } // namespace

    bool RHITexture::nativeCreate()
    {
        // allocate memory
        RHIDevice::memoryTextureCreate(this);

        if (hasShaderReadData())
        {
            uploadTexture(*this);
        }

        // transition layout
        {
            RHIImageLayout layout = RHIImageLayout::Max;
            if (m_usage & RHITextureViewUsageFlagBits::Rtv)
            {
                layout = RHIImageLayout::Attachment;
            }
            if (m_usage & RHITextureViewUsageFlagBits::Uav)
            {
                layout = RHIImageLayout::General;
            }
            if (m_usage & RHITextureViewUsageFlagBits::Srv)
            {
                layout = RHIImageLayout::ShaderRead;
            }

            if (RHICommandList* cmdList =
                    RHIDevice::cmdImmediateBegin(RHIQueueType::Graphics))
            {

                cmdList->insertBarrier(m_image, m_format, layout);

                RHIDevice::cmdImmediateSubmit(cmdList);
            }
        }

        // create image view
        createImageView(m_image, m_rtv, this);

        return true;
    }

} // namespace worse
