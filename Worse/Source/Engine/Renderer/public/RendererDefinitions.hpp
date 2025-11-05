#pragma once
#include "RHITexture.hpp"

#include <cstddef>
#include <string>

namespace Worse
{

    enum class RendererRasterizerState : Size
    {
        DepthPrepass,
        SolidCullBack,
        SolidCullNone,
        Wireframe,
        Max
    };

    enum class RendererDepthStencilState : Size
    {
        Off,
        ReadWrite,
        ReadEqual,
        ReadGreaterEqual,
        ReadLessEqual,
        Max
    };

    enum class RendererBlendState : Size
    {
        Off,
        Max
    };

    enum class RendererShader : Size
    {
        PlaceholderV,
        PlaceholderP,
        DepthPrepassV,
        DepthPrepassP,
        DepthLightV,
        DepthLightP,
        LineV,
        LineP,
        PointV,
        PointP,
        GBufferV,
        GBufferP,
        LightC,
        PostFXC,
        BloomLuminanceC,
        BloomUpscaleC,
        Max
    };

    enum class RendererTarget : Size
    {
        // 渲染目标
        SceneHDR,
        // 后处理目标
        ScreenHDR,
        // GBuffer
        GBufferPosition,
        GBufferAlbedo,
        GBufferNormal,
        GBufferMaterial,
        // bloom
        BloomInitial,
        BloomDownSampleStage0,
        BloomDownSampleStage1,
        BloomDownSampleStage2,
        BloomDownSampleStage3,
        BloomFinal,
        DepthGBuffer,
        DepthLight,
        Max,
    };

    // builtin textures
    enum class RendererTexture : Size
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
        Size index;
    };

} // namespace Worse