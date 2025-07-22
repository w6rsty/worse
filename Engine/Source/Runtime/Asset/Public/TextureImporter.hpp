#pragma once
#include "Types.hpp"
#include "RHITypes.hpp"

#include <memory>
#include <functional>
#include <filesystem>

namespace worse
{
    // Holds texture properties and a deferred copy function
    struct DeferredTextureCopy
    {
        using CopyFn = std::function<void(byte*)>;

        CopyFn deferredCopyFn;
        i32 width;
        i32 height;
        i32 depth;
        i32 layers;
        i32 mipLevels;

        usize size; // Size in bytes

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