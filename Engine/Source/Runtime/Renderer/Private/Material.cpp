#include "Log.hpp"
#include "Material.hpp"
#include "Renderer.hpp"

#include <vector>
#include <unordered_map>

namespace worse
{
    namespace
    {
        // Helper function to get texture index with fallback
        std::size_t getTextureIndex(
            const std::optional<AssetHandle>& handle,
            const std::unordered_map<AssetHandle, std::size_t>& textureIndexMap,
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
            return static_cast<std::size_t>(defaultTexture);
        }
    } // namespace

    // clang-format off
    void buildMaterials(
        ecs::QueryView<MeshMaterial> materialView,
        ecs::Resource<AssetServer> assetServer,
        ecs::ResourceArray<TextureWrite> textureWrites,
        ecs::ResourceArray<StandardMaterial> materials,
        ecs::ResourceArray<StandardMaterialGPU> materialGPUs
    )
    {
        // Validate inputs
        if (!assetServer || !textureWrites || !materials || !materialGPUs)
        {
            WS_LOG_ERROR("Material", "Invalid resource parameters in buildMaterials");
            return;
        }

        // make sure material textures are loaded
        assetServer->load();

        // generate material indices map and descritpor write data
        std::unordered_map<AssetHandle, std::size_t> textureIndexMap;
        {
            textureWrites->clear();
            textureWrites->data().reserve(assetServer->getLoadedCount());

            // skip renderer builtin textures
            std::size_t index = static_cast<std::size_t>(RendererTexture::Max);
            assetServer->eachAsset(
            [&index, &textureWrites, &textureIndexMap]
            (AssetHandle handle, RHITexture* texture)
            {
                textureWrites->add(texture, index);
                textureIndexMap.emplace(handle, index);
                ++index;
            });
        }

        materialGPUs->clear();
        materialGPUs->data().resize(materials->size());

        // Convert each CPU material to GPU format, maintaining index correspondence
        for (std::size_t i = 0; i < materials->size(); ++i)
        {
            StandardMaterial* material = materials.get(i);
            if (!material)
            {
                WS_LOG_ERROR("Material", "Null material at index {}", i);
                continue;
            }

            StandardMaterialGPU& data = materialGPUs->data()[i];
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

        Renderer::createMaterialBuffers(materialGPUs->data());
    }
    // clang-format on        

}
