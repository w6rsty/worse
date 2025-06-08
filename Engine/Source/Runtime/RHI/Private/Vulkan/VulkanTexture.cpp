#include "RHITexture.hpp"
#include "RHIDevice.hpp"
#include "RHICommandList.hpp"

namespace worse
{
    namespace
    {
        void createImageView(RHINativeHandle image, RHINativeHandle& imageView,
                             RHITexture* texture)
        {
            VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
            switch (texture->getType())
            {
            case RHITextureType::Texture2D:
                viewType = VK_IMAGE_VIEW_TYPE_2D;
                break;
            case RHITextureType::Texture2DArray:
                viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                break;
            case RHITextureType::Texture3D:
                viewType = VK_IMAGE_VIEW_TYPE_3D;
                break;
            case RHITextureType::TextureCube:
                viewType = VK_IMAGE_VIEW_TYPE_CUBE;
                break;
            default:
                WS_ASSERT_MSG(false, "Unsupported texture type for image view");
                break;
            }

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

        // transition layout
        {
            RHIImageLayout layout = RHIImageLayout::Max;
            if (m_usage & RHITextureUsageFlagBits::Rtv)
            {
                layout = RHIImageLayout::ColorAttachment;
            }
            if (m_usage & RHITextureUsageFlagBits::Uav)
            {
                layout = RHIImageLayout::General;
            }
            if (m_usage & RHITextureUsageFlagBits::Srv)
            {
                layout = RHIImageLayout::ShaderRead;
            }

            if (RHICommandList* cmdList =
                    RHIDevice::CmdImmediateBegin(RHIQueueType::Graphics))
            {

                cmdList->insertBarrier(m_image, m_format, layout);

                RHIDevice::CmdImmediateSubmit(cmdList);
            }
        }

        // create image view
        createImageView(m_image, m_rtv, this);

        return true;
    }

    void RHITexture::nativeDestroy()
    {
        RHIDevice::deletionQueueAdd(m_rtv);
        m_rtv = {};

        RHIDevice::deletionQueueAdd(m_image);
        m_image = {};
    }

} // namespace worse