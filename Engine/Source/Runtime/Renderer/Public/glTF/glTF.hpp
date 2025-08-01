#pragma once
#include "Mesh.hpp"
#include "Types.hpp"
#include "../Material.hpp"
#include "../Scene/Structure.hpp"

#include <vector>
#include <string>

namespace worse
{
    struct glTFMaterial
    {
        StandardMaterial data;
    };

    struct glTFSurface
    {
        u32 count;
        glTFMaterial* material;
    };

    struct glTFMesh
    {
        std::string name;
        std::vector<glTFSurface> surfaces;
        Mesh meshBuffer;
    };

    class glTFMeshNode final : public Node
    {
    public:
        glTFMesh* mesh;

        virtual void Draw(math::Matrix4 const& topMat, DrawContext& ctx) override;
    };

    struct glTFModel
    {
        // Logical nodes for tree hierarchy
        std::unordered_map<std::string, std::shared_ptr<glTFMeshNode>> nodes;
        // Top nodes for models in a single scene
        std::vector<std::shared_ptr<Node>> topNodes;

        std::unordered_map<std::string, glTFMesh*> meshes;
        std::unordered_map<std::string, glTFMaterial*> materials;
    };

    class glTFManager
    {
    public:
        glTFManager();
        ~glTFManager();

        glTFModel* load(std::string const& filepath);

    private:
        std::vector<glTFModel*> m_modelStorage;
    };

} // namespace worse