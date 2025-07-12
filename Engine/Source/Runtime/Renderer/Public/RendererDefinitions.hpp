#pragma once
#include "RHITexture.hpp"

#include <cstddef>
#include <string>

namespace worse
{

    enum class RendererRasterizerState : std::size_t
    {
        Solid,
        Wireframe,
        Max
    };

    enum class RendererDepthStencilState : std::size_t
    {
        Off,
        ReadWrite,
        ReadEqual,
        ReadGreaterEqual,
        ReadLessEqual,
        Max
    };

    enum class RendererBlendState : std::size_t
    {
        Off,
        Max
    };

    enum class RendererShader : std::size_t
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

    enum class RendererTarget : std::size_t
    {
        Render,
        Output,
        Depth,
        Max,
    };

    // builtin textures
    enum class RendererTexture : std::size_t
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
    enum class RendererPSO : std::size_t
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
        std::size_t index;
    };

} // namespace worse