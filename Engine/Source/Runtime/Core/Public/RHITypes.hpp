#pragma once

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

} // namespace worse