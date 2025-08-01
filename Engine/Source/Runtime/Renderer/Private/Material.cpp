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
            RendererTexture defaultTexture)
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
            return static_cast<usize>(defaultTexture);
        }
    } // namespace

    // clang-format off
    void buildMaterials(
        ecs::Resource<AssetServer> assetServer, // in
        ecs::ResourceArray<StandardMaterial> materials, // in
        ecs::ResourceArray<TextureWrite> textureWrites // out
    )
    {
        if (materials->empty())
        {
            // if no materials are defined, create a default material
            materials.add(StandardMaterial{});
        }

        // make sure material textures are loaded
        assetServer->load();

        // generate material indices map and descritpor write data
        std::unordered_map<AssetHandle, usize> textureIndexMap;
        {
            textureWrites->clear();
            textureWrites->data().reserve(assetServer->getLoadedCount());

            // skip renderer builtin textures
            usize index = static_cast<usize>(RendererTexture::Max);
            assetServer->eachTexture(
            [&index, &textureWrites, &textureIndexMap]
            (AssetHandle handle, RHITexture* texture)
            {
                textureWrites->add(texture, index);
                textureIndexMap.emplace(handle, index);
                ++index;
            });
        }

        materialGPUs.clear();
        materialGPUs.resize(materials->size());

        // Convert each CPU material to GPU format, maintaining index correspondence
        for (usize i = 0; i < materials->size(); ++i)
        {
            StandardMaterial* material = materials.get(i);
            if (!material)
            {
                WS_LOG_ERROR("Material", "Null material at index {}", i);
                continue;
            }

            StandardMaterialGPU& data = materialGPUs[i];
            data.albedo = material->albedo;
            
            // Map texture handles to indices using helper function
            data.albedoTextureIndex = getTextureIndex(material->albedoTexture, textureIndexMap, RendererTexture::DefaultAlbedo);
            data.normalTextureIndex = getTextureIndex(material->normalTexture, textureIndexMap, RendererTexture::DefaultNormal);
            
            data.metallic             = material->metallic;
            data.metallicTextureIndex = getTextureIndex(material->metallicTexture, textureIndexMap, RendererTexture::DefaultMetallic);
            
            data.roughness             = material->roughness;
            data.roughnessTextureIndex = getTextureIndex(material->roughnessTexture, textureIndexMap, RendererTexture::DefaultRoughness);
            
            data.ambientOcclusionTextureIndex = getTextureIndex(material->ambientOcclusionTexture, textureIndexMap, RendererTexture::DefaultAmbientOcclusion);
            
            data.emissive             = material->emissive;
            data.emissiveTextureIndex = getTextureIndex(material->emissiveTexture, textureIndexMap, RendererTexture::DefaultEmissive);
        }

        Renderer::createMaterialBuffers(materialGPUs);
    }
    // clang-format on        

}
