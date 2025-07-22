#pragma once
#include "RHITexture.hpp"

#include <cstddef>
#include <string>

namespace worse
{

    enum class RendererRasterizerState : usize
    {
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
        KuwaharaC,
        LineV,
        LineP,
        PointV,
        PointP,
        PBRV,
        PBRP,
        DistortionV,
        DistortionP,
        Max
    };

    enum class RendererTarget : usize
    {
        Render,
        Output,
        Depth,
        Max,
    };

    // builtin textures
    enum class RendererTexture : usize
    {
        Placeholder,
        DefaultAlbedo,
        DefaultNormal,
        DefaultMetallic,
        DefaultRoughness,
        DefaultAmbientOcclusion,
        DefaultEmissive,
        Max,
    };

    constexpr std::string renderTextureToString(RendererTexture texture)
    {
        switch (texture)
        {
            // clang-format off
        case RendererTexture::Placeholder:             return "Placeholder";
        case RendererTexture::DefaultNormal:           return "DefaultNormal";
        case RendererTexture::DefaultMetallic:         return "DefaultMetallic";
        case RendererTexture::DefaultRoughness:        return "DefaultRoughness";
        case RendererTexture::DefaultAmbientOcclusion: return "DefaultAmbientOcclusion";
        case RendererTexture::DefaultEmissive:         return "DefaultEmissive";
        default:                                       return "Unknown";
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