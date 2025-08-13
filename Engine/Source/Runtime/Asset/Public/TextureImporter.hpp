#pragma once
#include "Types.hpp"
#include "RHITypes.hpp"

#include <span>
#include <optional>
#include <functional>
#include <filesystem>

namespace worse
{
    /**
     * @brief 已加载纹理数据的内存引用视图
     */
    struct TextureLoadView
    {
        using CopyFn = std::function<void(byte*)>;

        // 获取已加载到内存中的纹理数据，然后释放内存
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
        static std::optional<TextureLoadView> fromFile(std::filesystem::path const& filepath);

        static std::optional<TextureLoadView> fromMemory(std::span<byte> data, std::string const& name);

        /**
         * @brief 将多个单通道纹理合并
         */
        static std::optional<TextureLoadView> combine(std::optional<TextureLoadView> r = std::nullopt,
                                                      std::optional<TextureLoadView> g = std::nullopt,
                                                      std::optional<TextureLoadView> b = std::nullopt,
                                                      std::optional<TextureLoadView> a = std::nullopt);
    };

} // namespace worse