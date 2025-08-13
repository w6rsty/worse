#pragma once
#include "AssetServer.hpp"
#include "RendererDefinitions.hpp"

#include "ECS/Resource.hpp"

namespace worse
{

    // ECS index
    struct MeshMaterial
    {
        usize index;
    };

    struct StandardMaterialGPU
    {
        u32 baseColorTextureIndex;
        u32 normalTextureIndex;
        u32 metallicRoughnessTextureIndex;
        u32 ambientOcclusionTextureIndex;
        u32 emissiveTextureIndex;
        f32 metallic;
        f32 roughness;
        f32 ambientOcclusion;

        math::Vector4 baseColor;
        math::Vector4 emissive;
    };

    /**
     * @brief 编排初始化中注册的材质
     *
     * @param assetServer 资源服务器
     * @param materials
     * @param textureWrites
     */
    void buildMaterials(
        ecs::Resource<AssetServer> assetServer,
        ecs::ResourceArray<StandardMaterial> materials,
        ecs::ResourceArray<TextureWrite> textureWrites);

} // namespace worse