#include "Log.hpp"
#include "Material.hpp"
#include "Renderer.hpp"

#include <vector>
#include <optional>
#include <unordered_map>

namespace worse
{
    namespace
    {
        std::vector<StandardMaterialGPU> materialGPUs;

        std::optional<usize> getTextureIndex(
            std::optional<AssetHandle> const& handle,
            std::unordered_map<AssetHandle, usize> const& textureIndexMap)
        {
            if (handle.has_value())
            {
                auto it = textureIndexMap.find(handle.value());
                if (it != textureIndexMap.end())
                {
                    return it->second;
                }
                // Log warning when texture not found in map
                WS_LOG_WARN(
                    "Material",
                    "Texture handle {} not found in texture map, using default",
                    handle.value());
            }

            return std::nullopt;
        }
    } // namespace

    void buildMaterials(
        ecs::Resource<AssetServer> assetServer,
        ecs::ResourceArray<StandardMaterial> materials,
        ecs::ResourceArray<TextureWrite> textureWrites)
    {
        if (materials->empty())
        {
            // if no materials are defined, create a default material
            materials.add(StandardMaterial{});
        }

        // make sure texture files are loaded
        assetServer->loadTexture();

        // generate material indices map and descritpor write data
        std::unordered_map<AssetHandle, usize> textureIndexMap;
        {
            textureWrites->clear();
            textureWrites->data().reserve(assetServer->getLoadedTextureCount());

            // skip renderer builtin textures
            usize index = static_cast<usize>(RendererTexture::Max);
            assetServer->eachTexture(
                [&index, &textureWrites, &textureIndexMap](AssetHandle handle, RHITexture* texture)
                {
                    // 纹理索引
                    textureWrites->add(texture, index);
                    // 保存索引映射，用于后续构建材质
                    textureIndexMap.emplace(handle, index);
                    ++index;
                });
        }

        materialGPUs.clear();
        // materials: ECS 中收集到的材质
        // assetServer: 资源服务器中加载的材质
        materialGPUs.resize(materials->size() + assetServer->getMaterialCount());

        // Convert each CPU material to GPU format, maintaining index correspondence
        for (usize i = 0; i < materials->size(); ++i)
        {
            StandardMaterial* materialECS = materials.get(i);

            StandardMaterialGPU& data = materialGPUs[i];
            data.flags                = 0;

            data.baseColor = materialECS->baseColor;

            if (getTextureIndex(materialECS->baseColorTexture, textureIndexMap).has_value())
            {
                data.baseColorTextureIndex = getTextureIndex(materialECS->baseColorTexture, textureIndexMap).value();
                data.flags |= 1 << 0;
            }
            if (getTextureIndex(materialECS->normalTexture, textureIndexMap).has_value())
            {
                data.normalTextureIndex = getTextureIndex(materialECS->normalTexture, textureIndexMap).value();
                data.flags |= 1 << 1;
            }
            data.metallic  = materialECS->metallic;
            data.roughness = materialECS->roughness;
            if (getTextureIndex(materialECS->metallicRoughnessTexture, textureIndexMap).has_value())
            {
                data.metallicRoughnessTextureIndex = getTextureIndex(materialECS->metallicRoughnessTexture, textureIndexMap).value();
                data.flags |= 1 << 2;
            }
            data.ambientOcclusion = materialECS->ambientOcclusion;
            if (getTextureIndex(materialECS->ambientOcclusionTexture, textureIndexMap).has_value())
            {
                data.ambientOcclusionTextureIndex = getTextureIndex(materialECS->ambientOcclusionTexture, textureIndexMap).value();
                data.flags |= 1 << 3;
            }
            data.emissive = materialECS->emissive;
            if (getTextureIndex(materialECS->emissiveTexture, textureIndexMap).has_value())
            {
                data.emissiveTextureIndex = getTextureIndex(materialECS->emissiveTexture, textureIndexMap).value();
                data.flags |= 1 << 4;
            }

            // encode flags
            if (data.baseColorTextureIndex)
            {
            }
        }

        u32 assetServerMaterialIndex = materials->size();
        assetServer->eachMaterial(
            [&](AssetHandle handle, MaterialAssetSlot& slot)
            {
                StandardMaterial const& material = slot.material;

                usize index               = assetServerMaterialIndex++;
                StandardMaterialGPU& data = materialGPUs[index];
                data.flags                = 0;

                data.baseColor                     = material.baseColor;
                if (getTextureIndex(material.baseColorTexture, textureIndexMap).has_value())
                {
                    data.baseColorTextureIndex = getTextureIndex(material.baseColorTexture, textureIndexMap).value();
                    data.flags |= 1 << 0;
                }
                if (getTextureIndex(material.normalTexture, textureIndexMap).has_value())
                {
                    data.normalTextureIndex = getTextureIndex(material.normalTexture, textureIndexMap).value();
                    data.flags |= 1 << 1;
                }
                data.metallic                      = material.metallic;
                data.roughness                     = material.roughness;
                if (getTextureIndex(material.metallicRoughnessTexture, textureIndexMap).has_value())
                {
                    data.metallicRoughnessTextureIndex = getTextureIndex(material.metallicRoughnessTexture, textureIndexMap).value();
                    data.flags |= 1 << 2;
                }
                data.ambientOcclusion              = material.ambientOcclusion;
                if (getTextureIndex(material.ambientOcclusionTexture, textureIndexMap).has_value())
                {
                    data.ambientOcclusionTextureIndex = getTextureIndex(material.ambientOcclusionTexture, textureIndexMap).value();
                    data.flags |= 1 << 3;
                }
                data.emissive                      = material.emissive;
                if (getTextureIndex(material.emissiveTexture, textureIndexMap).has_value())
                {
                    data.emissiveTextureIndex = getTextureIndex(material.emissiveTexture, textureIndexMap).value();
                    data.flags |= 1 << 4;
                }

                slot.index = index;
            });

        Renderer::createMaterialBuffers(materialGPUs);
    }

} // namespace worse
