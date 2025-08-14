#pragma once
#include "RHITexture.hpp"

#include <cstddef>
#include <string>

namespace worse
{

    enum class RendererRasterizerState : usize
    {
        DepthPrepass,
        Solid,
        Wireframe,
        Max
    };

    enum class RendererDepthStencilState : usize
    {
        Off,
        ReadWrite,
        ReadEqual,
        ReadGreaterEqual,
        ReadLessEqual,
        Max
    };

    enum class RendererBlendState : usize
    {
        Off,
        Max
    };

    enum class RendererShader : usize
    {
        PlaceholderV,
        PlaceholderP,
        DepthPrepassV,
        DepthPrepassP,
        LineV,
        LineP,
        PointV,
        PointP,
        PBRV,
        PBRP,
        PostFXC,
        BloomBrightFilterC,
        BloomUpscaleC,
        Max
    };

    enum class RendererTarget : usize
    {
        // 渲染目标
        SceneHDR,
        // 后处理目标
        ScreenHDR,
        // GBuffers
        GBufferNormal,
        GBufferAlbedo,
        BloomDownSampleStage0,
        BloomDownSampleStage1,
        BloomDownSampleStage2,
        BloomDownSampleStage3,
        BloomFinal,
        Depth,
        Max,
    };

    // builtin textures
    enum class RendererTexture : usize
    {
        Placeholder,
        DefaultAlbedo,
        DefaultNormal,
        DefaultMetallicRoughness,
        DefaultAmbientOcclusion,
        DefaultEmissive,
        Max,
    };

    constexpr std::string renderTextureToString(RendererTexture texture)
    {
        switch (texture)
        {
            // clang-format off
        case RendererTexture::Placeholder:              return "Placeholder";
        case RendererTexture::DefaultNormal:            return "DefaultNormal";
        case RendererTexture::DefaultMetallicRoughness: return "DefaultMetallicRoughness";
        case RendererTexture::DefaultAmbientOcclusion:  return "DefaultAmbientOcclusion";
        case RendererTexture::DefaultEmissive:          return "DefaultEmissive";
        default:                                        return "Unknown";
            // clang-format on
        }
    }

    struct TextureWrite
    {
        RHITexture* texture;
        usize index;
    };

} // namespace worse