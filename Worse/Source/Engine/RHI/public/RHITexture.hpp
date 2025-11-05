#pragma once
#include "base_type.hpp"
#include "bit_flag.hpp"
#include "RHIResource.hpp"

#include <span>
#include <filesystem>

namespace Worse
{

    WORSE_BEGIN_DECLARE_BIT_FLAG(RHITextureViewUsage, UInt)
    // clang-format off
    WORSE_DECLARE_FLAG_BIT(Unknown,             0)
    WORSE_DECLARE_FLAG_BIT(ShaderReadView,      1u << 0)
    WORSE_DECLARE_FLAG_BIT(UnorderedAccessView, 1u << 1)
    WORSE_DECLARE_FLAG_BIT(RenderTargetView,    1u << 2)
    WORSE_DECLARE_FLAG_BIT(DepthStencilView,    1u << 3)
    WORSE_DECLARE_FLAG_BIT(ClearOrBlit,         1u << 4)
    // clang-format off
    WORSE_END_DECLARE_BIT_FLAG(RHITextureViewUsage)

    struct RHITextureMip
    {
        std::vector<Byte> bytes;
    };

    struct RHITextureSlice
    {
        // TODO: only has mip 0 for now
        std::vector<RHITextureMip> mips;
    };

    class RHITexture : public RHIResource
    {
        friend class RHIDevice;

        Bool nativeCreate();

    public:
        RHITexture() = default;
        /**
         * @brief Create texture
         *
         * @note only accept mip 0 texture or texture array
         */
        RHITexture(RHITextureType const type, UInt const width, UInt const height,
                   UInt const depth, UInt const mipCount, RHIFormat const format,
                   RHITextureViewUsage::Flags const usageFlags,
                   std::vector<RHITextureSlice> data, std::string const& name);
        /**
         * @brief Load from file
         */
        RHITexture(std::filesystem::path const& path);
        /**
         * @brief Load from memory
         */
        RHITexture(std::span<Byte> data, std::string const& name);
        /**
         * @brief Assemble from multiple file
         */
        RHITexture(std::filesystem::path const& rPath,
                   std::filesystem::path const& gPath,
                   std::filesystem::path const& bPath,
                   std::filesystem::path const& aPath,
                   std::string const& name);
        ~RHITexture();

        RHIImageLayout getImageLayout() const;

        void convertImageLayout(RHICommandList* cmdList, RHIImageLayout const layout) const;

        Bool isFormatDepth() const;
        Bool isFormatStencil() const;

        Bool hasShaderReadData() const;
        RHITextureSlice const& getSlice(Size const arrayIndex) const;
        RHITextureMip const& getMip(Size const arrayIndex, Size const mipIndex) const;

        Bool isValid() const;

        // clang-format off
        RHITextureType             getType() const       { return m_type; }
        UInt                       getWidth() const      { return m_width; }
        UInt                       getHeight() const     { return m_height; }
        UInt                       getDepth() const      { return m_depth; }
        UInt                       getMipCount() const   { return m_mipCount; }
        RHIFormat                  getFormat() const     { return m_format; }
        RHITextureViewUsage::Flags getUsageFlags() const { return m_usageFlags; }
        RHINativeHandle            getView() const       { return m_rtv; }
        RHINativeHandle            getImage() const      { return m_image; }
        // clang-format on

    private:
        RHITextureType m_type = RHITextureType::Max;
        UInt m_width          = 0;
        UInt m_height         = 0;
        UInt m_depth          = 1;
        UInt m_mipCount       = 1;
        RHIFormat m_format    = RHIFormat::Max;
        RHITextureViewUsage::Flags m_usageFlags = RHITextureViewUsage::FlagBits::Unknown;

        // For texture array and cube map, we have multiple slices
        std::vector<RHITextureSlice> m_slices;

        RHINativeHandle m_rtv   = {};
        RHINativeHandle m_image = {};
    };

} // namespace Worse