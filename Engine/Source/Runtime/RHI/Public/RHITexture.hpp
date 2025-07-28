#pragma once
#include "Types.hpp"
#include "RHIResource.hpp"

#include <filesystem>

namespace worse
{

    WS_DEFINE_FLAGS(RHITextureView, u32);
    struct RHITextureViewFlagBits
    {
        // shader resource view, for shader read only
        static constexpr RHITextureViewFlags ShaderReadView{1u << 0};
        // unordered access view, for shader read/write
        static constexpr RHITextureViewFlags UnorderedAccessView{1u << 1};
        // render target view
        static constexpr RHITextureViewFlags RenderTargetView{1u << 2};
        // depth stencil view
        static constexpr RHITextureViewFlags DepthStencilView{1u << 3};
        static constexpr RHITextureViewFlags ClearOrBlit{1u << 4};
    };

    struct RHITextureMip
    {
        std::vector<byte> bytes;
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
        RHITexture(RHITextureType const type, u32 const width, u32 const height,
                   u32 const depth, u32 const mipCount, RHIFormat const format,
                   RHITextureViewFlags const usage,
                   std::vector<RHITextureSlice> data, std::string const& name);
        RHITexture(std::filesystem::path const& path);
        ~RHITexture();

        RHIImageLayout getImageLayout() const;
        void convertImageLayout(RHICommandList* cmdList,
                                RHIImageLayout const layout) const;

        bool isFormatDepth() const;
        bool isFormatStencil() const;

        bool hasShaderReadData() const;
        RHITextureSlice const& getSlice(usize const arrayIndex) const;
        RHITextureMip const& getMip(usize const arrayIndex,
                                    usize const mipIndex) const;

        bool isValid() const;

        // clang-format off
        RHITextureType      getType() const     { return m_type; }
        u32                 getWidth() const    { return m_width; }
        u32                 getHeight() const   { return m_height; }
        u32                 getDepth() const    { return m_depth; }
        u32                 getMipCount() const { return m_mipCount; }
        RHIFormat           getFormat() const   { return m_format; }
        RHITextureViewFlags getUsage() const    { return m_usage; }
        RHINativeHandle     getView() const     { return m_rtv; }
        RHINativeHandle     getImage() const    { return m_image; }
        // clang-format on

    private:
        RHITextureType m_type = RHITextureType::Max;
        u32 m_width           = 0;
        u32 m_height          = 0;
        u32 m_depth           = 1;
        u32 m_mipCount        = 1;
        RHIFormat m_format    = RHIFormat::Max;
        RHITextureViewFlags m_usage;

        // For texture array and cube map, we have multiple slices
        std::vector<RHITextureSlice> m_slices;

        RHINativeHandle m_rtv   = {};
        RHINativeHandle m_image = {};
    };

} // namespace worse