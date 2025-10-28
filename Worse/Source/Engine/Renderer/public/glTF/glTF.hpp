#pragma once
#include "fastgltf/core.hpp"

#include "Mesh.hpp"

#include "../Scene/Hierarchy.hpp"

#include <mutex>
#include <vector>
#include <string>
#include <memory>

namespace worse
{

    struct glTFSurface
    {
        u32 startIndex;
        u32 indexCount;
        AssetHandle material;
    };

    struct glTFMesh
    {
        std::string name;
        std::vector<glTFSurface> surfaces;
        std::unique_ptr<Mesh> mesh;
    };

    class glTFMeshNode final : public Node
    {
    public:
        std::shared_ptr<glTFMesh> mesh;

        void draw(math::Matrix4 const& topMat, DrawContext& ctx) override;
    };

    struct glTFModel
    {
        // 场景中顶级节点
        std::vector<std::shared_ptr<Node>> topNodes;
        // 场景中所有节点
        std::unordered_map<std::string, std::shared_ptr<glTFMeshNode>> nodes;
        std::unordered_map<std::string, std::shared_ptr<glTFMesh>> meshes;
        std::unordered_map<std::string, AssetHandle> textures;
    };

    class glTFManager
    {
    public:
        glTFManager(AssetServer& assetServer);
        ~glTFManager();

        glTFModel* load(std::string const& filepath, std::string const& modelName);

        glTFModel* getModel(std::string const& modelName);

    private:
        fastgltf::Parser m_parser{};
        mutable std::mutex m_mtxParser;

        AssetServer& m_assetServer;

        std::unordered_map<std::string, std::unique_ptr<glTFModel>> m_modelStorage;
    };

    // hack
    inline void drawModel(
        std::string const& modelName,
        math::Matrix4 const& xform,
        glTFManager& gltfManager,
        DrawContext& ctx)
    {
        if (glTFModel* model = gltfManager.getModel(modelName))
        {
            for (std::shared_ptr<Node> const& node : model->topNodes)
            {
                node->draw(xform, ctx);
            }
        }
    }

} // namespace worse