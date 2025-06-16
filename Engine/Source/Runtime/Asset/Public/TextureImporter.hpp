#pragma once
#include "RHITypes.hpp"

#include <memory>
#include <functional>
#include <filesystem>

namespace worse
{
    // Holds texture properties and a deferred copy function
    struct DeferredTextureCopy
    {
        using CopyFn = std::function<void(std::byte*)>;

        CopyFn deferredCopyFn;
        std::int32_t width;
        std::int32_t height;
        std::int32_t depth;
        std::int32_t layers;
        std::int32_t mipLevels;

        std::size_t size; // Size in bytes

        RHITextureType type;
        RHIFormat format;
    };

    class TextureImporter
    {
    public:
        // Load a texture file, return nullptr if failed
        static std::unique_ptr<DeferredTextureCopy>
        FromFile(std::filesystem::path const& filepath);
    };

} // namespace worse