#pragma once
#include "RHITexture.hpp"

#include <queue>
#include <mutex>
#include <memory>
#include <filesystem>
#include <functional>
#include <unordered_map>

namespace worse
{

    using AssetHandle = u64;

    enum class AssetState
    {
        Unloaded,
        Queued,
        Loaded,
        Failed
    };

    struct AssetSlot
    {
        std::shared_ptr<RHITexture> texture = nullptr;
        AssetState state                    = AssetState::Unloaded;
    };

    // support iamges only for now
    class AssetServer
    {
    public:
        AssetServer() = default;
        ~AssetServer();

        AssetHandle submitLoading(std::filesystem::path const& path);

        // try load assets in queue
        void load();
        void unload(AssetHandle const handle);
        void unloadAll();
        // Remove failed and unloaded assets
        void clean();

        bool isLoaded(AssetHandle const handle) const;
        AssetState getState(AssetHandle const handle) const;
        RHITexture* getTexture(AssetHandle handle) const;
        usize getLoadedCount() const;

        void eachAsset(std::function<void(AssetHandle, RHITexture*)> const&
                           callback) const;

    private:
        mutable std::mutex m_mtx;
        std::hash<std::filesystem::path> m_hasher;
        std::queue<std::filesystem::path> m_loadQueue;
        std::unordered_map<AssetHandle, AssetSlot> m_assets;
    };
} // namespace worse