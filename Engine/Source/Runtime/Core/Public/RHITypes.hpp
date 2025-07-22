#pragma once
#include "Math/Vector.hpp"

namespace worse
{

    enum class RHIFormat
    {
        // R
        R8Unorm,
        R8Uint,
        R16Unorm,
        R16Uint,
        R16Float,
        R32Uint,
        R32Float,
        // Rg
        R8G8Unorm,
        R16G16Float,
        R32G32Float,
        // Rgb
        R11G11B10Float,
        R32G32B32Float,
        // Rgba
        R8G8B8A8Unorm,
        B8R8G8A8Unorm,
        R10G10B10A2Unorm,
        R16G16B16A16Unorm,
        R16G16B16A16Snorm,
        R16G16B16A16Float,
        R32G32B32A32Float,
        // Depth
        D16Unorm,
        D32Float,
        D32FloatS8X24Uint,

        Max
    };

    enum class RHITextureType
    {
        Texture2D,
        Texture2DArray,
        Texture3D,
        TextureCube,
        Max
    };

    enum class RHIFilter
    {
        Nearest,
        Linear,
    };

    // =========================================================================
    // Vertex Structures
    // =========================================================================

    struct RHIVertexPos
    {
        math::Vector3 position = {0.0, 0.0, 0.0};
    };

    struct RHIVertexPosCol
    {
        math::Vector3 position = {0.0, 0.0, 0.0};
        math::Vector4 color    = {0.0, 0.0, 0.0, 0.0};
    };

    struct RHIVertexPosUv
    {
        math::Vector3 position = {0.0, 0.0, 0.0};
        math::Vector2 uv       = {0.0, 0.0};
    };

    struct RHIVertexPosUvNrmTan
    {
        math::Vector3 position = {0.0, 0.0, 0.0};
        math::Vector2 uv       = {0.0, 0.0};
        math::Vector3 normal   = {0.0, 0.0, 0.0};
        math::Vector3 tangent  = {0.0, 0.0, 0.0};
    };

    // 要求位置来构建包围盒
    template <typename T>
    concept RHIVertexConcept = requires(T t) {
        { t.position } -> std::same_as<math::Vector3&>;
    };

} // namespace worse