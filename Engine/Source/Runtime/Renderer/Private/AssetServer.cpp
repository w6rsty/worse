#include "Log.hpp"
#include "Definitions.hpp"
#include "Math/Hash.hpp"
#include "AssetServer.hpp"

namespace worse
{

    AssetServer::~AssetServer()
    {
        unloadAll();
    }

    AssetHandle AssetServer::addTexture(std::filesystem::path const& path)
    {
        std::lock_guard<std::mutex> lock(m_mtx);

        AssetHandle handle = m_hasher(path);

        auto it = m_assets.find(handle);
        if (it != m_assets.end())
        {
            AssetState state = it->second.state;
            if (state == AssetState::Queued || state == AssetState::Loaded)
            {
                return handle; // already queued or loaded
            }
        }

        m_loadQueue.push(path);
        m_assets.emplace(handle, TextureAssetSlot{.texture = nullptr, .state = AssetState::Queued});
        return handle;
    }

    AssetHandle AssetServer::addMaterial(StandardMaterial const& material)
    {
        std::lock_guard<std::mutex> lock(m_mtx);

        AssetHandle handle{};
        handle = math::hashCombine(handle, reinterpret_cast<u64 const&>(material.albedo.x));
        handle = math::hashCombine(handle, reinterpret_cast<u64 const&>(material.albedo.y));
        handle = math::hashCombine(handle, reinterpret_cast<u64 const&>(material.albedo.z));
        handle = math::hashCombine(handle, reinterpret_cast<u64 const&>(material.albedo.w));
        handle = math::hashCombine(handle, material.albedoTexture.value_or(AssetHandle{}));
        handle = math::hashCombine(handle, material.normalTexture.value_or(AssetHandle{}));
        handle = math::hashCombine(handle, material.metallicTexture.value_or(AssetHandle{}));
        handle = math::hashCombine(handle, material.metallic);
        handle = math::hashCombine(handle, material.roughnessTexture.value_or(AssetHandle{}));
        handle = math::hashCombine(handle, material.roughness);
        handle = math::hashCombine(handle, material.ambientOcclusionTexture.value_or(AssetHandle{}));
        handle = math::hashCombine(handle, material.emissiveTexture.value_or(AssetHandle{}));
        handle = math::hashCombine(handle, material.emissive.x);
        handle = math::hashCombine(handle, material.emissive.y);
        handle = math::hashCombine(handle, material.emissive.z);
        handle = math::hashCombine(handle, material.emissive.w);

        auto it = m_materials.find(handle);
        if (it != m_materials.end())
        {
            WS_LOG_WARN("AssetServer", "Material already exists with handle {}", handle);
            return handle;
        }

        m_materials.emplace(handle, MaterialAssetSlot{.material = std::move(material)});
        return handle;
    }

    void AssetServer::load()
    {
        std::lock_guard<std::mutex> lock(m_mtx);

        while (!m_loadQueue.empty())
        {
            std::filesystem::path path = m_loadQueue.front();
            m_loadQueue.pop();

            AssetHandle handle     = m_hasher(path);
            TextureAssetSlot& slot = m_assets[handle];
            WS_ASSERT_MSG(slot.state != AssetState::Loaded, "Asset already loaded");

            // Load the texture from the file system
            slot.texture = std::make_shared<RHITexture>(path);
            if (slot.texture->isValid())
            {
                slot.state = AssetState::Loaded;
            }
            else
            {
                slot.state   = AssetState::Failed;
                slot.texture = nullptr;
                WS_LOG_ERROR("AssetServer", "Failed to load texture {}", path.string());
            }
        }
    }

    bool AssetServer::isLoaded(AssetHandle const handle) const
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_assets.find(handle);
        return it != m_assets.end() && it->second.state == AssetState::Loaded;
    }

    AssetState AssetServer::getState(AssetHandle const handle) const
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_assets.find(handle);
        return it != m_assets.end() ? it->second.state : AssetState::Unloaded;
    }

    RHITexture* AssetServer::getTexture(AssetHandle handle) const
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_assets.find(handle);
        return (it != m_assets.end() && it->second.state == AssetState::Loaded)
                   ? it->second.texture.get()
                   : nullptr;
    }

    StandardMaterial const* AssetServer::getMaterial(AssetHandle handle) const
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_materials.find(handle);
        if (it != m_materials.end())
        {
            return &it->second.material;
        }
        return nullptr;
    }

    void AssetServer::unload(AssetHandle const handle)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_assets.find(handle);
        if (it != m_assets.end())
        {
            it->second.texture = nullptr;
            it->second.state   = AssetState::Unloaded;
        }
    }

    void AssetServer::unloadAll()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        for (auto& pair : m_assets)
        {
            pair.second.texture.reset();
            pair.second.state = AssetState::Unloaded;
        }
    }

    void AssetServer::cleanSlots()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_assets.begin();
        while (it != m_assets.end())
        {
            if ((it->second.state == AssetState::Failed) ||
                (it->second.state == AssetState::Unloaded))
            {
                it = m_assets.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    usize AssetServer::getLoadedCount() const
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return std::count_if(m_assets.begin(),
                             m_assets.end(),
                             [](const auto& pair)
                             {
                                 return pair.second.state == AssetState::Loaded;
                             });
    }

    void AssetServer::eachTexture(
        std::function<void(AssetHandle, RHITexture*)> const& callback) const
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        for (auto const& [handle, slot] : m_assets)
        {
            if (slot.state == AssetState::Loaded)
            {
                callback(handle, slot.texture.get());
            }
        }
    }

} // namespace worse