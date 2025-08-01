#pragma once
#include "RHITexture.hpp"

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

    // Renderer texture
    struct StandardMaterial
    {
        math::Vector4 albedo                     = math::Vector4::ONE();
        std::optional<AssetHandle> albedoTexture = std::nullopt;

        std::optional<AssetHandle> normalTexture = std::nullopt;

        f32 metallic                               = 1.0f;
        std::optional<AssetHandle> metallicTexture = std::nullopt;

        f32 roughness                               = 1.0f;
        std::optional<AssetHandle> roughnessTexture = std::nullopt;

        std::optional<AssetHandle> ambientOcclusionTexture = std::nullopt;

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
    };

    class AssetServer
    {
    public:
        AssetServer() = default;
        ~AssetServer();

        /**
         * @brief 提交纹理文件到加载队列
         *
         * @param path 纹理文件路径
         * @return AssetHandle 资源句柄
         */
        AssetHandle addTexture(std::filesystem::path const& path);

        /**
         * @brief 立即添加纹理资源到服务器
         */
        AssetHandle addMaterial(StandardMaterial const& material);

        /**
         * @brief 加载排队的资源
         */
        void load();
        void unload(AssetHandle const handle);
        void unloadAll();
        /**
         * @brief 清理加载失败或未加载的资源
         *
         */
        void cleanSlots();

        bool isLoaded(AssetHandle const handle) const;
        AssetState getState(AssetHandle const handle) const;
        RHITexture* getTexture(AssetHandle handle) const;
        StandardMaterial const* getMaterial(AssetHandle handle) const;
        usize getLoadedCount() const;

        /**
         * @brief 批量处理加载的纹理
         *
         * @param callback 对纹理的处理回调
         */
        void eachTexture(std::function<void(AssetHandle, RHITexture*)> const& callback) const;

    private:
        mutable std::mutex m_mtx;
        std::hash<std::filesystem::path> m_hasher;
        std::queue<std::filesystem::path> m_loadQueue;
        std::unordered_map<AssetHandle, TextureAssetSlot> m_assets;
        std::unordered_map<AssetHandle, MaterialAssetSlot> m_materials;
    };
} // namespace worse