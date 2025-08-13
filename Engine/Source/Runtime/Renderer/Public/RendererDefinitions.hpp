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
        PostFXC,
        LineV,
        LineP,
        PointV,
        PointP,
        PBRV,
        PBRP,
        Max
    };

    enum class RendererTarget : usize
    {
        Render,
        Output,
        GBufferNormal,
        GBufferAlbedo,
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

    // builtin pipeline
    enum class RendererPSO : usize
    {
        DepthPrepass,
        PBR,
        Wireframe,
        Point,
        PostProcessing,
        Max
    };

    struct TextureWrite
    {
        RHITexture* texture;
        usize index;
    };

} // namespace worse