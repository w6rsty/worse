#pragma once
#include "base_type.hpp"
#include "RHITypes.hpp"

#include <span>
#include <optional>
#include <functional>
#include <filesystem>

namespace Worse
{

    struct TextureLoadView
    {
        using CopyFn = std::function<void(Byte*)>;

        CopyFn deferredCopyFn;
        Int width;
        Int height;
        Int depth;
        Int layers;
        Int mipLevels;

        Size size; // Size in bytes

        RHITextureType type;
        RHIFormat format;
    };

    class TextureImporter
    {
    public:
        static std::optional<TextureLoadView> fromFile(std::filesystem::path const& filepath);

        static std::optional<TextureLoadView> fromMemory(std::span<Byte> data, std::string const& name);
        static std::optional<TextureLoadView> combine(std::optional<TextureLoadView> r = std::nullopt,
                                                      std::optional<TextureLoadView> g = std::nullopt,
                                                      std::optional<TextureLoadView> b = std::nullopt,
                                                      std::optional<TextureLoadView> a = std::nullopt);
    };

} // namespace Worse