#pragma once
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
        PBRV,
        PBRP,
        Max
    };

    constexpr std::string renderShaderToString(RendererShader shader)
    {
        switch (shader)
        {
            // clang-format off
        case RendererShader::PlaceholderV:  return "PlaceholderV";
        case RendererShader::PlaceholderP:  return "PlaceholderP";
        case RendererShader::DepthPrepassV: return "DepthPrepassV";
        case RendererShader::DepthPrepassP: return "DepthPrepassP";
        case RendererShader::KuwaharaC:     return "KuwaharaC";
        case RendererShader::PBRV:          return "PBRV";
        case RendererShader::PBRP:          return "PBRP";
        default:                            return "Unknown";
            // clang-format on
        }
    }

    enum class RendererTarget : std::size_t
    {
        Render,
        Output,
        Depth,
        Max,
    };

    enum class RendererTexture : std::size_t
    {
        Cornell,
        Placeholder,
        Max,
    };

    constexpr std::string renderTextureToString(RendererTexture texture)
    {
        switch (texture)
        {
            // clang-format off
        case RendererTexture::Cornell:     return "Cornell";
        case RendererTexture::Placeholder: return "Placeholder";
        default:                           return "Unknown";
            // clang-format on
        }
    }

} // namespace worse