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
        std::size_t index;
    };

    // Renderer texture
    struct StandardMaterial
    {
        math::Vector4 albedo                     = math::Vector4::ONE();
        std::optional<AssetHandle> albedoTexture = std::nullopt;

        std::optional<AssetHandle> normalTexture = std::nullopt;

        float metallic                             = 1.0f;
        std::optional<AssetHandle> metallicTexture = std::nullopt;

        float roughness                             = 1.0f;
        std::optional<AssetHandle> roughnessTexture = std::nullopt;

        std::optional<AssetHandle> ambientOcclusionTexture = std::nullopt;

        math::Vector4 emissive                     = math::Vector4::ZERO();
        std::optional<AssetHandle> emissiveTexture = std::nullopt;
    };

    struct StandardMaterialGPU
    {
        std::uint32_t albedoTextureIndex;
        std::uint32_t normalTextureIndex;
        std::uint32_t metallicTextureIndex;
        std::uint32_t roughnessTextureIndex;
        std::uint32_t ambientOcclusionTextureIndex;
        std::uint32_t emissiveTextureIndex;
        float metallic;
        float roughness;
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