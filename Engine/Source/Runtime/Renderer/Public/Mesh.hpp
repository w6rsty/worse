#pragma once
#include "Math/BoundingBox.hpp"
#include "Descriptor/RHIBuffer.hpp"

#include <memory>

namespace worse
{

    struct MeshLod
    {
        std::uint32_t vertexCount;
        std::uint32_t vertexOffset;
        std::uint32_t indexCount;
        std::uint32_t indexOffset;

        math::BoundingBox boundingBox;
    };

    struct SubMesh
    {
        std::vector<MeshLod> lods;
        bool isSolid = true; // indicates occlusion culling
    };

    class Mesh
    {
    public:
        Mesh();
        ~Mesh();

        // Release CPU resources
        void clear();

        // TODO: Now just lod0
        void addGeometry(std::vector<RHIVertexPosUvNrmTan> const& vertices,
                         std::vector<std::uint32_t> const& indices);

        void createGPUBuffers();

        // clang-format off
        RHIBuffer* getVertexBuffer() const { return m_vertexBuffer.get(); }
        RHIBuffer* getIndexBuffer() const { return m_indexBuffer.get(); }
        // clang-format on

    private:
        std::vector<RHIVertexPosUvNrmTan> m_vertices;
        std::vector<std::uint32_t> m_indices;
        std::vector<SubMesh> m_subMeshes;

        std::shared_ptr<RHIBuffer> m_vertexBuffer = nullptr;
        std::shared_ptr<RHIBuffer> m_indexBuffer  = nullptr;
    };

} // namespace worse