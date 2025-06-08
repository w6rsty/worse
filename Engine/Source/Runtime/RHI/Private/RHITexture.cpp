#include "RHITexture.hpp"
#include "RHICommandList.hpp"

namespace worse
{

    RHITexture::RHITexture(RHITextureType const type, std::uint32_t const width,
                           std::uint32_t const height,
                           std::uint32_t const depth, RHIFormat const format,
                           RHITextureUsageFlags const usage,
                           std::string const& name)
        : RHIResource(name)
    {
        m_type   = type;
        m_width  = width;
        m_height = height;
        m_depth  = depth;
        m_format = format;
        m_usage  = usage;

        WS_ASSERT(nativeCreate());
    }

    RHITexture::~RHITexture()
    {
        nativeDestroy();
    }

    RHIImageLayout RHITexture::getImageLayout() const
    {
        return m_image ? RHICommandList::getImageLayout(m_image)
                       : RHIImageLayout::Max;
    }

    bool RHITexture::isFormatDepth() const
    {
        return (m_format == RHIFormat::D16Unorm) ||
               (m_format == RHIFormat::D32Float) ||
               (m_format == RHIFormat::D32FloatS8X24Uint);
    }

    bool RHITexture::isFormatStencil() const
    {
        return m_format == RHIFormat::D32FloatS8X24Uint;
    }

} // namespace worse