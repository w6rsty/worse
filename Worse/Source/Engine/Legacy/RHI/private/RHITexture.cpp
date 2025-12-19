#include "logger/logger_macro.hpp"
#include "TextureImporter.hpp"
#include "RHIDevice.hpp"
#include "RHICommandList.hpp"
#include "RHITexture.hpp"

namespace Worse
{

    RHITexture::RHITexture(RHITextureType const type, UInt const width,
                           UInt const height, UInt const depth, UInt mipCount,
                           RHIFormat const format,
                           RHITextureViewUsage::Flags const usageFlags,
                           std::vector<RHITextureSlice> data,
                           std::string const& name)
        : RHIResource(name)
    {
        m_type       = type;
        m_width      = width;
        m_height     = height;
        m_depth      = depth;
        m_mipCount   = mipCount;
        m_format     = format;
        m_usageFlags = usageFlags;

        m_slices = data;

        if (!nativeCreate())
        {
            WORSE_LOG_ERROR("RHITexture", "Failed to create texture: {}", name);
        }
    }

    RHITexture::RHITexture(std::filesystem::path const& path)
    {
        if (std::optional<TextureLoadView> view = TextureImporter::fromFile(path))
        {
            m_name       = path.filename().string();
            m_type       = view->type;
            m_width      = view->width;
            m_height     = view->height;
            m_depth      = view->depth;
            m_mipCount   = view->mipLevels;
            m_format     = view->format;
            m_usageFlags = RHITextureViewUsage::FlagBits::ShaderReadView | RHITextureViewUsage::FlagBits::ClearOrBlit;

            m_slices.resize(view->layers);            // only 1 now, no array
            m_slices[0].mips.resize(view->mipLevels); // mip 0 only
            m_slices[0].mips[0].bytes.resize(view->size);

            view->deferredCopyFn(reinterpret_cast<Byte*>(m_slices[0].mips[0].bytes.data()));

            if (!nativeCreate())
            {
                WORSE_LOG_ERROR("RHITexture", "Failed to create texture from file: {}", path.string());
            }
        }
    }

    RHITexture::RHITexture(std::span<Byte> data, std::string const& name)
    {
        if (data.empty())
        {
            WORSE_LOG_ERROR("RHITexture", "Empty texture data");
            return;
        }

        if (std::optional<TextureLoadView> view = TextureImporter::fromMemory(data, name))
        {
            m_name       = name;
            m_type       = RHITextureType::Texture2D;
            m_width      = view->width;
            m_height     = view->height;
            m_depth      = view->depth;
            m_mipCount   = view->mipLevels;
            m_format     = view->format;
            m_usageFlags = RHITextureViewUsage::FlagBits::ShaderReadView | RHITextureViewUsage::FlagBits::ClearOrBlit;

            m_slices.resize(view->layers);            // only 1 now, no array
            m_slices[0].mips.resize(view->mipLevels); // mip
            m_slices[0].mips[0].bytes.resize(view->size);

            view->deferredCopyFn(reinterpret_cast<Byte*>(m_slices[0].mips[0].bytes.data()));

            if (!nativeCreate())
            {
                WORSE_LOG_ERROR("RHITexture", "Failed to create texture from memory");
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
            m_name       = name;
            m_type       = view->type;
            m_width      = view->width;
            m_height     = view->height;
            m_depth      = view->depth;
            m_mipCount   = view->mipLevels;
            m_format     = view->format;
            m_usageFlags = RHITextureViewUsage::FlagBits::ShaderReadView | RHITextureViewUsage::FlagBits::ClearOrBlit;

            m_slices.resize(view->layers);            // only 1 now, no array
            m_slices[0].mips.resize(view->mipLevels); // mip 0 only
            m_slices[0].mips[0].bytes.resize(view->size);

            view->deferredCopyFn(reinterpret_cast<Byte*>(m_slices[0].mips[0].bytes.data()));

            if (!nativeCreate())
            {
                WORSE_LOG_ERROR("RHITexture", "Failed to create combined texture: {}", name);
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
        cmdList->insertBarrier(m_image, m_format, layout, RHIPipelineStage::FlagBits::AllCommands, RHIAccessUsage::FlagBits::MemoryRead, RHIPipelineStage::FlagBits::AllCommands, RHIAccessUsage::FlagBits::MemoryWrite);
    }

    Bool RHITexture::isFormatDepth() const
    {
        return (m_format == RHIFormat::D16Unorm) ||
               (m_format == RHIFormat::D32Float) ||
               (m_format == RHIFormat::D32FloatS8X24Uint);
    }

    Bool RHITexture::isFormatStencil() const
    {
        return m_format == RHIFormat::D32FloatS8X24Uint;
    }

    Bool RHITexture::hasShaderReadData() const
    {
        // ensure as least one slice, one layer and one mip level data
        return (!m_slices.empty() && !m_slices[0].mips.empty() &&
                !m_slices[0].mips[0].bytes.empty());
    }

    RHITextureSlice const& RHITexture::getSlice(Size const arrayIndex) const
    {
        WORSE_ASSERT(arrayIndex < m_slices.size());

        return m_slices[arrayIndex];
    }

    RHITextureMip const& RHITexture::getMip(Size const arrayIndex, Size const mipIndex) const
    {
        WORSE_ASSERT(arrayIndex < m_slices.size());
        WORSE_ASSERT(mipIndex < m_mipCount);

        return m_slices[arrayIndex].mips[mipIndex];
    }

    Bool RHITexture::isValid() const
    {
        return m_image && m_rtv;
    }

} // namespace Worse