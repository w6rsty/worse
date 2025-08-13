#include "Log.hpp"
#include "Material.hpp"
#include "Renderer.hpp"

#include <vector>
#include <unordered_map>

namespace worse
{
    namespace
    {
        std::vector<StandardMaterialGPU> materialGPUs;

        // Helper function to get texture index with fallback
        usize getTextureIndex(
            std::optional<AssetHandle> const& handle,
            std::unordered_map<AssetHandle, usize> const& textureIndexMap,
            RendererTexture fallback)
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
            return static_cast<usize>(fallback);
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

            data.baseColor = materialECS->baseColor;

            data.baseColorTextureIndex         = getTextureIndex(materialECS->baseColorTexture, textureIndexMap, RendererTexture::DefaultAlbedo);
            data.normalTextureIndex            = getTextureIndex(materialECS->normalTexture, textureIndexMap, RendererTexture::DefaultNormal);
            data.metallic                      = materialECS->metallic;
            data.metallicRoughnessTextureIndex = getTextureIndex(materialECS->metallicRoughnessTexture, textureIndexMap, RendererTexture::DefaultMetallicRoughness);
            data.roughness                     = materialECS->roughness;
            data.ambientOcclusionTextureIndex  = getTextureIndex(materialECS->ambientOcclusionTexture, textureIndexMap, RendererTexture::DefaultAmbientOcclusion);
            data.emissive                      = materialECS->emissive;
            data.emissiveTextureIndex          = getTextureIndex(materialECS->emissiveTexture, textureIndexMap, RendererTexture::DefaultEmissive);
        }

        u32 assetServerMaterialIndex = materials->size();
        assetServer->eachMaterial(
            [&](AssetHandle handle, MaterialAssetSlot& slot)
            {
                StandardMaterial const& material = slot.material;

                usize index               = assetServerMaterialIndex++;
                StandardMaterialGPU& data = materialGPUs[index];

                data.baseColor                     = material.baseColor;
                data.baseColorTextureIndex         = getTextureIndex(material.baseColorTexture, textureIndexMap, RendererTexture::DefaultAlbedo);
                data.normalTextureIndex            = getTextureIndex(material.normalTexture, textureIndexMap, RendererTexture::DefaultNormal);
                data.metallic                      = material.metallic;
                data.metallicRoughnessTextureIndex = getTextureIndex(material.metallicRoughnessTexture, textureIndexMap, RendererTexture::DefaultMetallicRoughness);
                data.roughness                     = material.roughness;
                data.ambientOcclusionTextureIndex  = getTextureIndex(material.ambientOcclusionTexture, textureIndexMap, RendererTexture::DefaultAmbientOcclusion);
                data.emissive                      = material.emissive;
                data.emissiveTextureIndex          = getTextureIndex(material.emissiveTexture, textureIndexMap, RendererTexture::DefaultEmissive);

                slot.index = index;
            });

        Renderer::createMaterialBuffers(materialGPUs);
    }

} // namespace worse
