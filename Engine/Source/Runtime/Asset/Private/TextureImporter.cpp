#include "Log.hpp"
#include "FileSystem.hpp"
#include "TextureImporter.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "gli/load.hpp"
#include "gli/format.hpp"
#include "gli/texture.hpp"

namespace worse
{

    namespace
    {
        RHITextureType rhiTextureType(gli::target target)
        {
            // clang-format off
            switch (target)
            {
            case gli::TARGET_2D:       return RHITextureType::Texture2D;
            case gli::TARGET_2D_ARRAY: return RHITextureType::Texture2DArray;
            case gli::TARGET_3D:       return RHITextureType::Texture3D;
            case gli::TARGET_CUBE:     return RHITextureType::TextureCube;
            default:                   return RHITextureType::Max;
            }
            // clang-format on
        }

        RHIFormat rhiFormat(gli::format format)
        {
            // clang-format off
            switch (format)
            {
            // R
            case gli::FORMAT_R8_UNORM_PACK8:            return RHIFormat::R8Unorm;
            case gli::FORMAT_R8_UINT_PACK8:             return RHIFormat::R8Uint;
            case gli::FORMAT_R16_UNORM_PACK16:          return RHIFormat::R16Unorm;
            case gli::FORMAT_R16_UINT_PACK16:           return RHIFormat::R16Uint;
            case gli::FORMAT_R32_UINT_PACK32:           return RHIFormat::R32Uint;
            case gli::FORMAT_R32_SFLOAT_PACK32:         return RHIFormat::R32Float;
            // RG
            case gli::FORMAT_RG8_UNORM_PACK8:           return RHIFormat::R8G8Unorm;
            case gli::FORMAT_RG16_SFLOAT_PACK16:        return RHIFormat::R16G16Float;
            case gli::FORMAT_RG32_SFLOAT_PACK32:        return RHIFormat::R32G32Float;
            // RGB
            case gli::FORMAT_RG11B10_UFLOAT_PACK32:     return RHIFormat::R11G11B10Float;
            case gli::FORMAT_RGB32_SFLOAT_PACK32:       return RHIFormat::R32G32B32Float;
            // RGBA
            case gli::FORMAT_RGBA8_UNORM_PACK8:         return RHIFormat::R8G8B8A8Unorm;
            case gli::FORMAT_BGRA8_UNORM_PACK8:         return RHIFormat::B8R8G8A8Unorm;
            case gli::FORMAT_RGB10A2_UNORM_PACK32:      return RHIFormat::R10G10B10A2Unorm;
            case gli::FORMAT_RGBA16_UNORM_PACK16:       return RHIFormat::R16G16B16A16Unorm;
            case gli::FORMAT_RGBA16_SNORM_PACK16:       return RHIFormat::R16G16B16A16Snorm;
            case gli::FORMAT_RGBA16_SFLOAT_PACK16:      return RHIFormat::R16G16B16A16Float;
            case gli::FORMAT_RGBA32_SFLOAT_PACK32:      return RHIFormat::R32G32B32A32Float;
            // Depth
            case gli::FORMAT_D16_UNORM_PACK16:          return RHIFormat::D16Unorm;
            case gli::FORMAT_D32_SFLOAT_PACK32:         return RHIFormat::D32Float;
            case gli::FORMAT_D32_SFLOAT_S8_UINT_PACK64: return RHIFormat::D32FloatS8X24Uint;
            default:
                return RHIFormat::Max;
            }
            // clang-format on
        }

        std::unique_ptr<DeferredTextureCopy>
        loadDDS(std::filesystem::path const& path)
        {
            gli::texture texture = gli::load(path.string());
            if (texture.empty() || (texture.size() == 0))
            {
                WS_LOG_WARN("Asset",
                            "Failed to load texture: {}",
                            path.string());
                return nullptr;
            }

            auto textureData       = std::make_unique<DeferredTextureCopy>();
            textureData->width     = texture.extent().x;
            textureData->height    = texture.extent().y;
            textureData->depth     = texture.extent().z;
            textureData->layers    = texture.layers();
            textureData->mipLevels = texture.levels();
            textureData->type      = rhiTextureType(texture.target());
            textureData->format    = rhiFormat(texture.format());
            textureData->size      = texture.size();

            if (textureData->type == RHITextureType::Max)
            {
                WS_LOG_ERROR("Asset",
                             "Failed to load texture. Unsupported type");
                return nullptr;
            }

            if (textureData->format == RHIFormat::Max)
            {
                WS_LOG_ERROR("Asset",
                             "Failed to load texture. Unsupported format");
                return nullptr;
            }

            textureData->deferredCopyFn =
                [texture = std::move(texture)](std::byte* dst)
            {
                std::memcpy(dst, texture.data(), texture.size());
            };

            return textureData;
        }

        std::unique_ptr<DeferredTextureCopy>
        loadCommon(std::filesystem::path const& path)
        {
            int width, height, originalChannels;
            // Force load as RGBA (4 channels) to match RHIFormat::R8G8B8A8Unorm
            constexpr int desiredChannels = 4;
            stbi_set_flip_vertically_on_load(true);
            stbi_uc* data = stbi_load(path.string().c_str(),
                                      &width,
                                      &height,
                                      &originalChannels,
                                      desiredChannels);
            if (!data)
            {
                WS_LOG_ERROR("Asset",
                             "Failed to load texture: {}",
                             path.string());
                return nullptr;
            }

            auto textureData       = std::make_unique<DeferredTextureCopy>();
            textureData->width     = width;
            textureData->height    = height;
            textureData->depth     = 1; // 2D textures
            textureData->layers    = 1; // Single layer
            textureData->mipLevels = 1; // No mipmaps
            textureData->type      = RHITextureType::Texture2D;
            textureData->format    = RHIFormat::R8G8B8A8Unorm;

            // Now size matches the format (always 4 channels)
            textureData->size = width * height * desiredChannels;
            textureData->deferredCopyFn =
                [data, size = textureData->size](std::byte* dst)
            {
                std::memcpy(dst, data, size);
                stbi_image_free(data);
            };

            return textureData;
        }
    } // namespace

    std::unique_ptr<DeferredTextureCopy>
    TextureImporter::FromFile(std::filesystem::path const& path)
    {

        if (!FileSystem::isSupportedImage(path))
        {
            WS_LOG_ERROR("Asset", "Failed to load texture. Unsupported format");
            return {};
        }

        std::unique_ptr<DeferredTextureCopy> textureData = nullptr;

        if (path.extension() == ".dds")
        {
            textureData = loadDDS(path);
        }
        else
        {
            textureData = loadCommon(path);
        }

        WS_LOG_INFO("Asset",
                    "Loaded texture: {} ({}x{}x{}, layers: {}, mip: {})",
                    path.string(),
                    textureData->width,
                    textureData->height,
                    textureData->depth,
                    textureData->layers,
                    textureData->mipLevels);

        return textureData;
    }

} // namespace worse