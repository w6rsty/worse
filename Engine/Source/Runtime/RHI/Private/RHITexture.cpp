#include "TextureImporter.hpp"
#include "RHIDevice.hpp"
#include "RHICommandList.hpp"
#include "RHITexture.hpp"

namespace worse
{

    RHITexture::RHITexture(RHITextureType const type, std::uint32_t const width,
                           std::uint32_t const height,
                           std::uint32_t const depth, std::uint32_t mipCount,
                           RHIFormat const format,
                           RHITextureViewUsageFlags const usage,
                           std::vector<RHITextureSlice> data,
                           std::string const& name)
        : RHIResource(name)
    {
        m_type     = type;
        m_width    = width;
        m_height   = height;
        m_depth    = depth;
        m_mipCount = mipCount;
        m_format   = format;
        m_usage    = usage;

        m_slices = data;

        WS_ASSERT(nativeCreate());
    }

    RHITexture::RHITexture(std::filesystem::path const& path)
    {
        auto data = TextureImporter::FromFile(path);
        if (!data)
        {
            WS_LOG_ERROR("RHITexture", "Failed to load texture");
            return;
        }

        m_name     = path.filename().string();
        m_type     = data->type;
        m_width    = data->width;
        m_height   = data->height;
        m_depth    = data->depth;
        m_mipCount = data->mipLevels;
        m_format   = data->format;
        m_usage    = RHITextureViewUsageFlagBits::Srv |
                  RHITextureViewUsageFlagBits::ClearOrBlit;

        m_slices.resize(data->layers);            // only 1 now, no array
        m_slices[0].mips.resize(data->mipLevels); // mip 0 only
        m_slices[0].mips[0].bytes.resize(data->size);

        data->deferredCopyFn(
            reinterpret_cast<std::byte*>(m_slices[0].mips[0].bytes.data()));

        WS_ASSERT(nativeCreate());
    }

    RHITexture::~RHITexture()
    {
        RHIDevice::deletionQueueAdd(m_rtv);
        m_rtv = {};

        RHIDevice::deletionQueueAdd(m_image);
        m_image = {};
    }

    RHIImageLayout RHITexture::getImageLayout() const
    {
        return m_image ? RHICommandList::getImageLayout(m_image)
                       : RHIImageLayout::Max;
    }

    void RHITexture::convertImageLayout(RHICommandList* cmdList,
                                        RHIImageLayout const layout) const
    {
        cmdList->insertBarrier(m_image, m_format, layout);
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

    bool RHITexture::hasShaderReadData() const
    {
        // ensure as least one slice, one layer and one mip level data
        return (!m_slices.empty() && !m_slices[0].mips.empty() &&
                !m_slices[0].mips[0].bytes.empty());
    }

    RHITextureSlice const&
    RHITexture::getSlice(std::size_t const arrayIndex) const
    {
        WS_ASSERT(arrayIndex < m_slices.size());

        return m_slices[arrayIndex];
    }

    RHITextureMip const& RHITexture::getMip(std::size_t const arrayIndex,
                                            std::size_t const mipIndex) const
    {
        WS_ASSERT(arrayIndex < m_slices.size());
        WS_ASSERT(mipIndex < m_mipCount);

        return m_slices[arrayIndex].mips[mipIndex];
    }

    bool RHITexture::isValid() const
    {
        return m_image && m_rtv;
    }

} // namespace worse