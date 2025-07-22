#pragma once
#include "AssetServer.hpp"
#include "RendererDefinitions.hpp"

#include "ECS/Resource.hpp"
#include "ECS/QueryView.hpp"

#include <optional>

namespace worse
{
    // ECS index
    struct MeshMaterial
    {
        usize index;
    };

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

    struct StandardMaterialGPU
    {
        u32 albedoTextureIndex;
        u32 normalTextureIndex;
        u32 metallicTextureIndex;
        u32 roughnessTextureIndex;
        u32 ambientOcclusionTextureIndex;
        u32 emissiveTextureIndex;
        f32 metallic;
        f32 roughness;
        math::Vector4 albedo;
        math::Vector4 emissive;
    };

    // clang-format off
    void buildMaterials(
        ecs::QueryView<MeshMaterial> materialView,
        ecs::Resource<AssetServer> assetServer,
        ecs::ResourceArray<TextureWrite> textureWrites,
        ecs::ResourceArray<StandardMaterial> materials,
        ecs::ResourceArray<StandardMaterialGPU> materialGPUs
    );
    // clang-format on 

} // namespace worse