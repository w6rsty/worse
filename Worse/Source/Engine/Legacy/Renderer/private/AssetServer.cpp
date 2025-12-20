#include "math/hash.hpp"
#include "logger/logger_macro.hpp"
#include "Platform.hpp"
#include "AssetServer.hpp"

namespace Worse
{

    AssetServer::AssetServer()
    {
        std::filesystem::path const textureDir = std::filesystem::path{Worse::EngineDirectory} / "Binary/Textures";
        m_errorTextureHandle                   = addTexture(textureDir / "no_texture.png");
    }

    AssetServer::~AssetServer()
    {
        unloadAll();
    }

    AssetHandle AssetServer::addTexture(
        std::filesystem::path const& path,
        LoadStrategy strategy)
    {
        std::hash<std::filesystem::path> hasher;
        AssetHandle handle = hasher(path);

        std::lock_guard<std::mutex> lock(m_mtxTexture);

        auto it = m_textures.find(handle);
        if (it != m_textures.end())
        {
            AssetState state = it->second.state;
            if (state == AssetState::Queued || state == AssetState::Loaded)
            {
                return handle; // already queued or loaded
            }
        }

        if (strategy == LoadStrategy::Immediate)
        {
            std::shared_ptr<RHITexture> texture = std::make_shared<RHITexture>(path);
            if (texture->isValid())
            {
                m_textures.emplace(handle, TextureAssetSlot{.texture = std::move(texture), .state = AssetState::Loaded});
            }
            else
            {
                m_textures.emplace(handle, TextureAssetSlot{.texture = nullptr, .state = AssetState::Failed});
                WORSE_LOG_ERROR("AssetServer", "Failed to load texture {}", path.string());
            }
        }
        else
        {
            m_loadQueue.push(path);
            m_textures.emplace(handle, TextureAssetSlot{.texture = nullptr, .state = AssetState::Queued});
        }

        return handle;
    }

    AssetHandle AssetServer::addTextureMetallicRoughness(
        std::filesystem::path const& pathMetallic,
        std::filesystem::path const& pathRoughness)
    {
        std::hash<std::filesystem::path> hasher;
        AssetHandle handle = {};
        handle             = math::hashCombine(handle, hasher(pathMetallic));
        handle             = math::hashCombine(handle, hasher(pathRoughness));

        std::lock_guard<std::mutex> lock(m_mtxTexture);
        auto it = m_textures.find(handle);
        if (it != m_textures.end())
        {
            return handle; // already exists
        }

        std::string name                    = pathMetallic.filename().string() + "_" + pathRoughness.filename().string();
        std::shared_ptr<RHITexture> texture = std::make_shared<RHITexture>(
            pathMetallic,
            pathRoughness,
            std::filesystem::path{},
            std::filesystem::path{},
            name);
        if (texture->isValid())
        {
            m_textures.emplace(handle, TextureAssetSlot{.texture = std::move(texture), .state = AssetState::Loaded});
        }
        else
        {
            m_textures.emplace(handle, TextureAssetSlot{.texture = nullptr, .state = AssetState::Failed});
            WORSE_LOG_ERROR("AssetServer", "Failed to load metallic/roughness texture: {} and {}", pathMetallic.string(), pathRoughness.string());
        }

        return handle;
    }

    AssetHandle AssetServer::addTexture(std::span<Byte> data, std::string const& name)
    {
        if (data.empty())
        {
            WORSE_LOG_WARN("AssetServer", "Empty texture data");
            return AssetHandle{};
        }

        std::hash<std::string> hasher;
        AssetHandle handle = hasher(name);

        std::lock_guard<std::mutex> lock(m_mtxTexture);

        auto it = m_textures.find(handle);
        if (it != m_textures.end())
        {
            return handle;
        }

        std::shared_ptr<RHITexture> texture = std::make_shared<RHITexture>(data, name);
        if (texture->isValid())
        {
            m_textures.emplace(handle, TextureAssetSlot{.texture = std::move(texture), .state = AssetState::Loaded});
        }
        else
        {
            m_textures.emplace(handle, TextureAssetSlot{.texture = nullptr, .state = AssetState::Failed});
            WORSE_LOG_ERROR("AssetServer", "Failed to load texture");
        }

        return handle;
    }

    AssetHandle AssetServer::addMaterial(StandardMaterial const& material)
    {
        std::lock_guard<std::mutex> lock(m_mtxMaterial);

        AssetHandle handle{};
        handle = math::hashCombine(handle, reinterpret_cast<ULong const&>(material.baseColor.x));
        handle = math::hashCombine(handle, reinterpret_cast<ULong const&>(material.baseColor.y));
        handle = math::hashCombine(handle, reinterpret_cast<ULong const&>(material.baseColor.z));
        handle = math::hashCombine(handle, reinterpret_cast<ULong const&>(material.baseColor.w));
        handle = math::hashCombine(handle, material.baseColorTexture.value_or(AssetHandle{}));
        handle = math::hashCombine(handle, material.normalTexture.value_or(AssetHandle{}));
        handle = math::hashCombine(handle, material.metallicRoughnessTexture.value_or(AssetHandle{}));
        handle = math::hashCombine(handle, material.metallic);
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
            WORSE_LOG_WARN("AssetServer", "Material already exists with handle {}", handle);
            return handle;
        }

        m_materials.emplace(handle, MaterialAssetSlot{.material = std::move(material)});
        return handle;
    }

    void AssetServer::loadTexture()
    {
        std::lock_guard<std::mutex> lock(m_mtxTexture);

        while (!m_loadQueue.empty())
        {
            std::filesystem::path path = m_loadQueue.front();
            m_loadQueue.pop();

            std::hash<std::filesystem::path> hasher;
            AssetHandle handle     = hasher(path);
            TextureAssetSlot& slot = m_textures[handle];
            WORSE_ASSERT_MSG(slot.state != AssetState::Loaded, "Asset already loaded");

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
                WORSE_LOG_ERROR("AssetServer", "Failed to load texture {}", path.string());
            }
        }
    }

    Bool AssetServer::isLoaded(AssetHandle const handle) const
    {
        std::lock_guard<std::mutex> lock(m_mtxTexture);
        auto it = m_textures.find(handle);
        return it != m_textures.end() && it->second.state == AssetState::Loaded;
    }

    AssetState AssetServer::getState(AssetHandle const handle) const
    {
        std::lock_guard<std::mutex> lock(m_mtxTexture);
        auto it = m_textures.find(handle);
        return it != m_textures.end() ? it->second.state : AssetState::Unloaded;
    }

    RHITexture* AssetServer::getTexture(AssetHandle handle) const
    {
        std::lock_guard<std::mutex> lock(m_mtxTexture);
        auto it = m_textures.find(handle);
        return (it != m_textures.end() && it->second.state == AssetState::Loaded)
                   ? it->second.texture.get()
                   : nullptr;
    }

    StandardMaterial const* AssetServer::getMaterial(AssetHandle handle) const
    {
        std::lock_guard<std::mutex> lock(m_mtxMaterial);
        auto it = m_materials.find(handle);
        if (it != m_materials.end())
        {
            return &it->second.material;
        }
        return nullptr;
    }

    UInt AssetServer::getMaterialIndex(AssetHandle handle) const
    {
        std::lock_guard<std::mutex> lock(m_mtxMaterial);
        auto it = m_materials.find(handle);
        if (it != m_materials.end())
        {
            return it->second.index.value_or(0);
        }
        return 0;
    }

    void AssetServer::unloadTexture(AssetHandle const handle)
    {
        std::lock_guard<std::mutex> lock(m_mtxTexture);
        auto it = m_textures.find(handle);
        if (it != m_textures.end())
        {
            it->second.texture = nullptr;
            it->second.state   = AssetState::Unloaded;
        }
    }

    void AssetServer::unloadMaterial(AssetHandle const handle)
    {
        std::lock_guard<std::mutex> lock(m_mtxMaterial);
        auto it = m_materials.find(handle);
        if (it != m_materials.end())
        {
            m_materials.erase(it);
        }
    }

    void AssetServer::unloadAll()
    {
        {
            std::lock_guard<std::mutex> lock(m_mtxTexture);
            for (auto& pair : m_textures)
            {
                pair.second.texture.reset();
                pair.second.state = AssetState::Unloaded;
            }
        }
        {
            std::lock_guard<std::mutex> lock(m_mtxMaterial);
            m_materials.clear();
        }
    }

    void AssetServer::cleanSlots()
    {
        std::lock_guard<std::mutex> lock(m_mtxTexture);
        auto it = m_textures.begin();
        while (it != m_textures.end())
        {
            if ((it->second.state == AssetState::Failed) ||
                (it->second.state == AssetState::Unloaded))
            {
                it = m_textures.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    Size AssetServer::getLoadedTextureCount() const
    {
        std::lock_guard<std::mutex> lock(m_mtxTexture);
        return std::count_if(
            m_textures.begin(),
            m_textures.end(),
            [](const auto& pair)
            {
                return pair.second.state == AssetState::Loaded;
            });
    }

    Size AssetServer::getMaterialCount() const
    {
        std::lock_guard<std::mutex> lock(m_mtxMaterial);
        return m_materials.size();
    }

    void AssetServer::eachTexture(
        std::function<void(AssetHandle, RHITexture*)> const& callback) const
    {
        std::lock_guard<std::mutex> lock(m_mtxTexture);
        for (auto const& [handle, slot] : m_textures)
        {
            if (slot.state == AssetState::Loaded)
            {
                callback(handle, slot.texture.get());
            }
        }
    }

    void AssetServer::eachMaterial(std::function<void(AssetHandle, MaterialAssetSlot&)> const& callback)
    {
        std::lock_guard<std::mutex> lock(m_mtxMaterial);
        for (auto& [handle, slot] : m_materials)
        {
            callback(handle, slot);
        }
    }

} // namespace Worse