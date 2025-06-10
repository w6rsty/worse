#pragma once
#include "Types.hpp"
#include "RHIResource.hpp"

#include <cstdint>

namespace worse
{

    WS_DEFINE_FLAGS(RHITextureUsage, std::uint32_t);
    struct RHITextureUsageFlagBits
    {
        // shader resource view, for shader read only
        static constexpr RHITextureUsageFlags Srv{1u << 0};
        // unordered access view, for shader read/write
        static constexpr RHITextureUsageFlags Uav{1u << 1};
        // render target view
        static constexpr RHITextureUsageFlags Rtv{1u << 2};
        // depth stencil view
        static constexpr RHITextureUsageFlags Dsv{1u << 3};
        static constexpr RHITextureUsageFlags ClearOrBlit{1u << 4};
    };

    class RHITexture : public RHIResource
    {
        bool nativeCreate();
        void nativeDestroy();

    public:
        RHITexture() = default;
        RHITexture(RHITextureType const type, std::uint32_t const width,
                   std::uint32_t const height, std::uint32_t const depth,
                   RHIFormat const format, RHITextureUsageFlags const usage,
                   std::string const& name);
        ~RHITexture();

        RHIImageLayout getImageLayout() const;
        void convertImageLayout(RHICommandList* cmdList, RHIImageLayout layout);

        bool isFormatDepth() const;
        bool isFormatStencil() const;

        // clang-format off
        RHITextureType getType() const        { return m_type; }
        std::uint32_t getWidth() const        { return m_width; }
        std::uint32_t getHeight() const       { return m_height; }
        std::uint32_t getDepth() const        { return m_depth; }
        RHIFormat getFormat() const           { return m_format; }
        RHITextureUsageFlags getUsage() const { return m_usage; }
        RHINativeHandle getRtv() const        { return m_rtv; }
        RHINativeHandle getImage() const      { return m_image; }
        // RHIDevice::memoryTextureCreate set this handle
        void setHandle(RHINativeHandle handle) { m_image = handle; }
        // clang-format on

    private:
        RHITextureType m_type  = RHITextureType::Max;
        std::uint32_t m_width  = 0;
        std::uint32_t m_height = 0;
        std::uint32_t m_depth  = 1;
        RHIFormat m_format     = RHIFormat::Max;
        RHITextureUsageFlags m_usage;

        RHINativeHandle m_rtv   = {};
        RHINativeHandle m_image = {};
    };

} // namespace worse