#include "TextureImporter.hpp"
#include "RHIDevice.hpp"
#include "RHICommandList.hpp"
#include "RHITexture.hpp"

namespace worse
{

    RHITexture::RHITexture(RHITextureType const type, u32 const width,
                           u32 const height, u32 const depth, u32 mipCount,
                           RHIFormat const format,
                           RHITextureViewFlags const usage,
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

        if (!nativeCreate())
        {
            WS_LOG_ERROR("RHITexture", "Failed to create texture: {}", name);
        }
    }

    RHITexture::RHITexture(std::filesystem::path const& path)
    {
        if (std::optional<TextureLoadView> view = TextureImporter::fromFile(path))
        {
            m_name     = path.filename().string();
            m_type     = view->type;
            m_width    = view->width;
            m_height   = view->height;
            m_depth    = view->depth;
            m_mipCount = view->mipLevels;
            m_format   = view->format;
            m_usage    = RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit;

            m_slices.resize(view->layers);            // only 1 now, no array
            m_slices[0].mips.resize(view->mipLevels); // mip 0 only
            m_slices[0].mips[0].bytes.resize(view->size);

            view->deferredCopyFn(reinterpret_cast<byte*>(m_slices[0].mips[0].bytes.data()));

            if (!nativeCreate())
            {
                WS_LOG_ERROR("RHITexture", "Failed to create texture from file: {}", path.string());
            }
        }
    }

    RHITexture::RHITexture(std::span<byte> data, std::string const& name)
    {
        if (data.empty())
        {
            WS_LOG_ERROR("RHITexture", "Empty texture data");
            return;
        }

        if (std::optional<TextureLoadView> view = TextureImporter::fromMemory(data, name))
        {
            m_name     = name;
            m_type     = RHITextureType::Texture2D;
            m_width    = view->width;
            m_height   = view->height;
            m_depth    = view->depth;
            m_mipCount = view->mipLevels;
            m_format   = view->format;
            m_usage    = RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit;

            m_slices.resize(view->layers);            // only 1 now, no array
            m_slices[0].mips.resize(view->mipLevels); // mip
            m_slices[0].mips[0].bytes.resize(view->size);

            view->deferredCopyFn(reinterpret_cast<byte*>(m_slices[0].mips[0].bytes.data()));

            if (!nativeCreate())
            {
                WS_LOG_ERROR("RHITexture", "Failed to create texture from memory");
            }
        }
    }

    RHITexture::RHITexture(std::filesystem::path const& rPath,
                           std::filesystem::path const& gPath,
                           std::filesystem::path const& bPath,
                           std::filesystem::path const& aPath,
                           std::string const& name)
    {
        if (std::optional<TextureLoadView> view = TextureImporter::combine(
                TextureImporter::fromFile(rPath),
                TextureImporter::fromFile(gPath),
                TextureImporter::fromFile(bPath),
                TextureImporter::fromFile(aPath)))
        {
            m_name     = name;
            m_type     = view->type;
            m_width    = view->width;
            m_height   = view->height;
            m_depth    = view->depth;
            m_mipCount = view->mipLevels;
            m_format   = view->format;
            m_usage    = RHITextureViewFlagBits::ShaderReadView | RHITextureViewFlagBits::ClearOrBlit;

            m_slices.resize(view->layers);            // only 1 now, no array
            m_slices[0].mips.resize(view->mipLevels); // mip 0 only
            m_slices[0].mips[0].bytes.resize(view->size);

            view->deferredCopyFn(reinterpret_cast<byte*>(m_slices[0].mips[0].bytes.data()));

            if (!nativeCreate())
            {
                WS_LOG_ERROR("RHITexture", "Failed to create combined texture: {}", name);
            }
        }
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
        return m_image ? RHICommandList::getImageLayout(m_image) : RHIImageLayout::Max;
    }

    void RHITexture::convertImageLayout(RHICommandList* cmdList, RHIImageLayout const layout) const
    {
        cmdList->insertBarrier(m_image, m_format, layout, RHIPipelineStageFlagBits::AllCommands, RHIAccessFlagBits::MemoryRead, RHIPipelineStageFlagBits::AllCommands, RHIAccessFlagBits::MemoryWrite);
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

    RHITextureSlice const& RHITexture::getSlice(usize const arrayIndex) const
    {
        WS_ASSERT(arrayIndex < m_slices.size());

        return m_slices[arrayIndex];
    }

    RHITextureMip const& RHITexture::getMip(usize const arrayIndex, usize const mipIndex) const
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