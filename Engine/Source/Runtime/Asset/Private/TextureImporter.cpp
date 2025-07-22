#include "Log.hpp"
#include "FileSystem.hpp"
#include "TextureImporter.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace worse
{

    namespace
    {

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
                [data, size = textureData->size](byte* dst)
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
        if (!FileSystem::isFileExists(path))
        {
            WS_LOG_ERROR("Asset",
                         "Failed to load texture. File {} not found",
                         path.string());
            return {};
        }

        if (!FileSystem::isSupportedImage(path))
        {
            WS_LOG_ERROR("Asset", "Failed to load texture. Unsupported format");
            return {};
        }

        std::unique_ptr<DeferredTextureCopy> textureData = nullptr;

        textureData = loadCommon(path);

        WS_LOG_INFO(
            "Asset",
            "Loaded texture: ({:>4}x{:>4}x{:>2}, layers: {:>2}, mip: {:>2}) {}",
            textureData->width,
            textureData->height,
            textureData->depth,
            textureData->layers,
            textureData->mipLevels,
            path.string());

        return textureData;
    }

} // namespace worse