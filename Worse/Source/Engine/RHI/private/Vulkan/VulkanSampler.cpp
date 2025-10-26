#include "RHISampler.hpp"

#include <limits>

namespace worse
{

    void RHISampler::nativeCreate()
    {
        VkSamplerCreateInfo infoSampler = {};
        infoSampler.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        infoSampler.magFilter           = vulkanFilter(m_filterMag);
        infoSampler.minFilter           = vulkanFilter(m_filterMin);
        infoSampler.mipmapMode          = vulkanSamplerMipmapMode(m_filterMipmap);
        infoSampler.addressModeU        = vulkanSamplerAddressMode(m_addressMode);
        infoSampler.addressModeV        = vulkanSamplerAddressMode(m_addressMode);
        infoSampler.addressModeW        = vulkanSamplerAddressMode(m_addressMode);

        infoSampler.mipLodBias       = m_mipLodBias;
        infoSampler.anisotropyEnable = m_anisotroy > 0.0f;
        infoSampler.maxAnisotropy    = m_anisotroy;
        infoSampler.compareEnable    = m_enableCompare;
        infoSampler.compareOp        = vulkanCompareOp(m_compareOp);
        infoSampler.minLod           = 0.0f;
        infoSampler.maxLod           = std::numeric_limits<f32>::max();
        infoSampler.borderColor      = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

        VkSampler sampler = VK_NULL_HANDLE;
        WS_ASSERT_VK(vkCreateSampler(RHIContext::device, &infoSampler, nullptr, &sampler));
        m_handle = RHINativeHandle{sampler, RHINativeHandleType::Sampler};
    }

} // namespace worse