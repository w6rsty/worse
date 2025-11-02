#include "Log.hpp"
#include "FileSystem.hpp"
#include "TextureImporter.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Worse
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
            textureData.deferredCopyFn = [data, size = textureData.size](Byte* dst)
            {
                std::memcpy(dst, data, size);
                stbi_image_free(data);
            };

            return std::make_optional(std::move(textureData));
        }

        std::optional<TextureLoadView>
        loadFromMemoryCommon(std::span<Byte> data)
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
            textureData.deferredCopyFn = [imgData, size = textureData.size](Byte* dst)
            {
                std::memcpy(dst, imgData, size);
                stbi_image_free(imgData);
            };

            return std::make_optional(std::move(textureData));
        }
    } // namespace

    std::optional<TextureLoadView> TextureImporter::fromFile(std::filesystem::path const& path)
    {
        if (path.empty())
        {
            return std::nullopt;
        }

        if (!FileSystem::isFileExists(path))
        {
            WS_LOG_WARN("Asset", "Failed to load texture. File {} not found", path.string());
            return {};
        }

        if (!FileSystem::isSupportedImage(path))
        {
            WS_LOG_ERROR("Asset", "Failed to load texture. Unsupported format");
            return {};
        }

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

    std::optional<TextureLoadView> TextureImporter::fromMemory(std::span<Byte> data, std::string const& name)
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

    std::optional<TextureLoadView> TextureImporter::combine(std::optional<TextureLoadView> r,
                                                            std::optional<TextureLoadView> g,
                                                            std::optional<TextureLoadView> b,
                                                            std::optional<TextureLoadView> a)
    {
        if (!r && !g && !b && !a)
        {
            return std::nullopt;
        }

        TextureLoadView const& ref = r ? *r : g ? *g
                                          : b   ? *b
                                                : *a;

        auto checkMatch = [&](const std::optional<TextureLoadView>& opt) -> Bool
        {
            if (!opt)
            {
                return true;
            }
            return (opt->width == ref.width) && (opt->height == ref.height) && (opt->depth == ref.depth);
        };

        if (!checkMatch(r) || !checkMatch(g) || !checkMatch(b) || !checkMatch(a))
        {
            return std::nullopt;
        }

        TextureLoadView out;
        out.width     = ref.width;
        out.height    = ref.height;
        out.depth     = 1;
        out.layers    = 1;
        out.mipLevels = 1;
        out.type      = ref.type;
        out.format    = RHIFormat::R8G8B8A8Unorm;
        out.size      = static_cast<Size>(out.width) * out.height * 4;

        out.deferredCopyFn = [r, g, b, a, out](Byte* dst)
        {
            std::vector<Byte> bufR, bufG, bufB, bufA;

            auto loadIf = [&](std::optional<TextureLoadView> const& opt, std::vector<Byte>& buf)
            {
                if (!opt)
                {
                    return;
                }
                buf.resize(opt->size);
                opt->deferredCopyFn(buf.data());
            };

            loadIf(r, bufR);
            loadIf(g, bufG);
            loadIf(b, bufB);
            loadIf(a, bufA);

            Size pixelCount = static_cast<Size>(out.width) * out.height;
            for (Size i = 0; i < pixelCount; ++i)
            {
                dst[i * 4 + 0] = r ? bufR[i * 4 + 0] : Byte{0};
                dst[i * 4 + 1] = g ? bufG[i * 4 + 0] : Byte{0};
                dst[i * 4 + 2] = b ? bufB[i * 4 + 0] : Byte{0};
                dst[i * 4 + 3] = a ? bufA[i * 4 + 0] : Byte{0};
            }
        };

        return out;
    }

} // namespace Worse