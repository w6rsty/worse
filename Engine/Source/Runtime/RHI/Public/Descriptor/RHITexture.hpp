#pragma once
#include "Types.hpp"
#include "RHIResource.hpp"

#include <cstdint>
#include <filesystem>

namespace worse
{

    WS_DEFINE_FLAGS(RHITextureViewUsage, std::uint32_t);
    struct RHITextureViewUsageFlagBits
    {
        // shader resource view, for shader read only
        static constexpr RHITextureViewUsageFlags Srv{1u << 0};
        // unordered access view, for shader read/write
        static constexpr RHITextureViewUsageFlags Uav{1u << 1};
        // render target view
        static constexpr RHITextureViewUsageFlags Rtv{1u << 2};
        // depth stencil view
        static constexpr RHITextureViewUsageFlags Dsv{1u << 3};
        static constexpr RHITextureViewUsageFlags ClearOrBlit{1u << 4};
    };

    struct RHITextureMip
    {
        std::vector<std::byte> bytes;
    };

    struct RHITextureSlice
    {
        // TODO: only has mip 0 for now
        std::vector<RHITextureMip> mips;
    };

    class RHITexture : public RHIResource
    {
        friend class RHIDevice;

        bool nativeCreate();

    public:
        RHITexture() = default;
        // only accept mip 0 texture or texture array
        RHITexture(RHITextureType const type, std::uint32_t const width,
                   std::uint32_t const height, std::uint32_t const depth,
                   std::uint32_t const mipCount, RHIFormat const format,
                   RHITextureViewUsageFlags const usage,
                   std::vector<RHITextureSlice> data, std::string const& name);
        RHITexture(std::filesystem::path const& path);
        ~RHITexture();

        RHIImageLayout getImageLayout() const;
        void convertImageLayout(RHICommandList* cmdList,
                                RHIImageLayout const layout) const;

        bool isFormatDepth() const;
        bool isFormatStencil() const;

        bool hasShaderReadData() const;
        RHITextureSlice const& getSlice(std::size_t const arrayIndex) const;
        RHITextureMip const& getMip(std::size_t const arrayIndex,
                                    std::size_t const mipIndex) const;

        // clang-format off
        RHITextureType       getType() const       { return m_type; }
        std::uint32_t        getWidth() const      { return m_width; }
        std::uint32_t        getHeight() const     { return m_height; }
        std::uint32_t        getDepth() const      { return m_depth; }
        std::uint32_t        getMipCount() const   { return m_mipCount; }
        RHIFormat            getFormat() const     { return m_format; }
        RHITextureViewUsageFlags getUsage() const  { return m_usage; }
        RHINativeHandle      getRtv() const        { return m_rtv; }
        RHINativeHandle      getImage() const      { return m_image; }
        // clang-format on

    private:
        RHITextureType m_type    = RHITextureType::Max;
        std::uint32_t m_width    = 0;
        std::uint32_t m_height   = 0;
        std::uint32_t m_depth    = 1;
        std::uint32_t m_mipCount = 1;
        RHIFormat m_format       = RHIFormat::Max;
        RHITextureViewUsageFlags m_usage;

        // For texture array and cube map, we have multiple slices
        std::vector<RHITextureSlice> m_slices;

        RHINativeHandle m_rtv   = {};
        RHINativeHandle m_image = {};
    };

} // namespace worse