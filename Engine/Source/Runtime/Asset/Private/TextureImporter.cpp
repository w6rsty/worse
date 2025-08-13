#include "Log.hpp"
#include "FileSystem.hpp"
#include "TextureImporter.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace worse
{

    namespace
    {
        std::optional<TextureLoadView>
        loadFromFileCommon(std::filesystem::path const& path)
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
                WS_LOG_ERROR("Asset", "Failed to load texture: {}", path.string());
                return std::nullopt;
            }

            TextureLoadView textureData;
            textureData.width     = width;
            textureData.height    = height;
            textureData.depth     = 1; // 2D textures
            textureData.layers    = 1; // Single layer
            textureData.mipLevels = 1; // No mipmaps
            textureData.type      = RHITextureType::Texture2D;
            textureData.format    = RHIFormat::R8G8B8A8Unorm;

            // Now size matches the format (always 4 channels)
            textureData.size           = width * height * desiredChannels;
            textureData.deferredCopyFn = [data, size = textureData.size](byte* dst)
            {
                std::memcpy(dst, data, size);
                stbi_image_free(data);
            };

            return std::make_optional(std::move(textureData));
        }

        std::optional<TextureLoadView>
        loadFromMemoryCommon(std::span<byte> data)
        {
            if (data.empty())
            {
                WS_LOG_ERROR("Asset", "Empty texture data");
                return std::nullopt;
            }

            int width, height, originalChannels;
            // Force load as RGBA (4 channels) to match RHIFormat::R8G8B8A8Unorm
            constexpr int desiredChannels = 4;
            // stbi_set_flip_vertically_on_load(true);
            stbi_uc* imgData = stbi_load_from_memory(
                reinterpret_cast<stbi_uc*>(data.data()),
                static_cast<int>(data.size()),
                &width,
                &height,
                &originalChannels,
                desiredChannels);
            if (!imgData)
            {
                WS_LOG_ERROR("Asset", "Failed to load texture from memory");
                return std::nullopt;
            }

            TextureLoadView textureData;
            textureData.width     = width;
            textureData.height    = height;
            textureData.depth     = 1; // 2D textures
            textureData.layers    = 1; // Single layer
            textureData.mipLevels = 1; // No mipmaps
            textureData.type      = RHITextureType::Texture2D;
            textureData.format    = RHIFormat::R8G8B8A8Unorm;

            // Now size matches the format (always 4 channels)
            textureData.size           = width * height * desiredChannels;
            textureData.deferredCopyFn = [imgData, size = textureData.size](byte* dst)
            {
                std::memcpy(dst, imgData, size);
                stbi_image_free(imgData);
            };

            return std::make_optional(std::move(textureData));
        }
    } // namespace

    std::optional<TextureLoadView>
    TextureImporter::fromFile(std::filesystem::path const& path)
    {
        // 检查文件是否存在
        if (!FileSystem::isFileExists(path))
        {
            WS_LOG_ERROR("Asset", "Failed to load texture. File {} not found", path.string());
            return {};
        }

        // 检查文件格式是否支持
        if (!FileSystem::isSupportedImage(path))
        {
            WS_LOG_ERROR("Asset", "Failed to load texture. Unsupported format");
            return {};
        }

        // 尝试加载纹理数据
        if (std::optional<TextureLoadView> textureData = std::move(loadFromFileCommon(path)))
        {
            WS_LOG_INFO(
                "Asset",
                "Loaded texture: {} ({:>4}x{:>4}x{:>2}, layers: {:>2}, mip: {:>2})",
                path.string(),
                textureData->width,
                textureData->height,
                textureData->depth,
                textureData->layers,
                textureData->mipLevels);

            return std::move(textureData);
        }

        return std::nullopt;
    }

    std::optional<TextureLoadView>
    TextureImporter::fromMemory(std::span<byte> data, std::string const& name)
    {
        if (std::optional<TextureLoadView> textureData = std::move(loadFromMemoryCommon(data)))
        {
            WS_LOG_INFO(
                "Asset",
                "Loaded texture: {} ({:>4}x{:>4}x{:>2}, layers: {:>2}, mip: {:>2})",
                name,
                textureData->width,
                textureData->height,
                textureData->depth,
                textureData->layers,
                textureData->mipLevels);

            return std::move(textureData);
        }

        return std::nullopt;
    }

} // namespace worse