#pragma once
#include "RHITexture.hpp"

#include <span>
#include <queue>
#include <mutex>
#include <memory>
#include <optional>
#include <filesystem>
#include <functional>
#include <unordered_map>

namespace worse
{

    using AssetHandle = u64;

    struct StandardMaterial
    {
        math::Vector4 baseColor                     = math::Vector4::ONE();
        std::optional<AssetHandle> baseColorTexture = std::nullopt;

        std::optional<AssetHandle> normalTexture = std::nullopt;

        f32 metallic                                        = 0.0f;
        std::optional<AssetHandle> metallicRoughnessTexture = std::nullopt;

        f32 roughness = 1.0f;

        std::optional<AssetHandle> ambientOcclusionTexture = std::nullopt;
        f32 ambientOcclusion                               = 1.0f;

        math::Vector4 emissive                     = math::Vector4::ZERO();
        std::optional<AssetHandle> emissiveTexture = std::nullopt;
    };

    enum class AssetState
    {
        Unloaded,
        Queued,
        Loaded,
        Failed
    };

    struct TextureAssetSlot
    {
        std::shared_ptr<RHITexture> texture = nullptr;
        AssetState state                    = AssetState::Unloaded;
    };

    struct MaterialAssetSlot
    {
        StandardMaterial material;
        u32 index;
    };

    class AssetServer
    {
    public:
        enum class LoadStrategy
        {
            Immediate,
            Deferred
        };

        AssetServer();
        ~AssetServer();

        /**
         * @brief 提交纹理文件到加载队列
         *
         * @param path 纹理文件路径
         * @param strategy 加载策略
         * @return AssetHandle 资源句柄
         */
        AssetHandle addTexture(std::filesystem::path const& path, LoadStrategy strategy = LoadStrategy::Deferred);

        /**
         * @brief 使用内存中的数据立即创建纹理
         *
         * @param data 纹理数据
         * @return AssetHandle 资源句柄
         */
        AssetHandle addTexture(std::span<byte> data, std::string const& name);

        /**
         * @brief 立即添加纹理资源到服务器
         */
        AssetHandle addMaterial(StandardMaterial const& material);
        void setMaterialIndex(AssetHandle handle, u32 index);

        /**
         * @brief 加载排队的纹理资源
         */
        void
        loadTexture();
        /**
         * @brief 释放纹理资源
         */
        void unloadTexture(AssetHandle const handle);
        /**
         * @brief 释放材质资源
         */
        void unloadMaterial(AssetHandle const handle);
        /**
         * @brief 释放所有资源
         */
        void unloadAll();
        /**
         * @brief 清理加载失败或未加载的资源
         */
        void cleanSlots();

        bool isLoaded(AssetHandle const handle) const;
        AssetState getState(AssetHandle const handle) const;
        RHITexture* getTexture(AssetHandle handle) const;
        StandardMaterial const* getMaterial(AssetHandle handle) const;
        u32 getMaterialIndex(AssetHandle handle) const;
        usize getLoadedTextureCount() const;
        usize getMaterialCount() const;

        /**
         * @brief 批量处理加载的纹理
         *
         * @param callback 对纹理的处理回调
         */
        void eachTexture(std::function<void(AssetHandle, RHITexture*)> const& callback) const;
        void eachMaterial(std::function<void(AssetHandle, MaterialAssetSlot&)> const& callback);

        // clang-format off
        AssetHandle getErrorTexture() const { return m_errorTextureHandle; }
        // clang-format on

    private:
        std::queue<std::filesystem::path> m_loadQueue;
        mutable std::mutex m_mtxTexture;
        std::unordered_map<AssetHandle, TextureAssetSlot> m_textures;
        mutable std::mutex m_mtxMaterial;
        std::unordered_map<AssetHandle, MaterialAssetSlot> m_materials;

        AssetHandle m_errorTextureHandle;
    };
} // namespace worse