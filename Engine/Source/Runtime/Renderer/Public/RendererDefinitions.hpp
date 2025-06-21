#pragma once
#include <cstddef>

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
        Max
    };

    enum class RendererBlendState : std::size_t
    {
        Off,
        Max
    };

    enum class RendererShader : std::size_t
    {
        TestC,
        PlaceholderV,
        PlaceholderP,
        PBRV,
        PBRP,
        Max
    };

    enum class RendererTarget : std::size_t
    {
        Render,
        Output,
        Max,
    };

    enum class RendererTexture : std::size_t
    {
        TestA,
        TestB,
        Placeholder,
        Max,
    };

} // namespace worse