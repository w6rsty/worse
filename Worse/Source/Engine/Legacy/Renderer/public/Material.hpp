#pragma once
#include "AssetServer.hpp"
#include "RendererDefinitions.hpp"

#include "ECS/Resource.hpp"

namespace Worse
{

    // ECS index
    struct MeshMaterial
    {
        Size index;
    };

    struct StandardMaterialGPU
    {
        UInt baseColorTextureIndex;
        UInt normalTextureIndex;
        UInt metallicRoughnessTextureIndex;
        UInt ambientOcclusionTextureIndex;

        UInt emissiveTextureIndex;
        Float metallic;
        Float roughness;
        Float ambientOcclusion;

        Vector4 baseColor;

        Vector4 emissive;

        UInt flags;
        UInt padding[3];
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

} // namespace Worse