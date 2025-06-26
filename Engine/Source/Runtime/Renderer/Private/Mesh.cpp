#include "Mesh.hpp"

namespace worse
{

    Mesh::Mesh()
    {
    }

    Mesh::~Mesh()
    {
        m_indexBuffer.reset();
        m_vertexBuffer.reset();
    }

    void Mesh::clear()
    {
        m_vertices.clear();
        m_indices.clear();
        m_subMeshes.clear();
    }

    void Mesh::addGeometry(std::vector<RHIVertexPosUvNrmTan> const& vertices,
                           std::vector<std::uint32_t> const& indices)
    {
        SubMesh subMesh;

        MeshLod lod0;
        lod0.vertexCount  = static_cast<std::uint32_t>(vertices.size());
        lod0.vertexOffset = static_cast<std::uint32_t>(m_vertices.size());
        lod0.indexCount   = static_cast<std::uint32_t>(indices.size());
        lod0.indexOffset  = static_cast<std::uint32_t>(m_indices.size());
        lod0.boundingBox  = math::BoundingBox(vertices);

        subMesh.lods.push_back(lod0);

        m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
        m_indices.insert(m_indices.end(), indices.begin(), indices.end());

        m_subMeshes.push_back(subMesh);
    }

    void Mesh::createGPUBuffers()
    {
        if (m_vertices.empty() || m_indices.empty())
        {
            WS_LOG_WARN("Mesh", "Incomplete mesh data");
            return;
        }

        m_vertexBuffer =
            std::make_shared<RHIBuffer>(RHIBufferUsageFlagBits::Vertex,
                                        sizeof(RHIVertexPosUvNrmTan),
                                        m_vertices.size(),
                                        m_vertices.data(),
                                        false,
                                        "MeshVertexBuffer");
        m_indexBuffer =
            std::make_shared<RHIBuffer>(RHIBufferUsageFlagBits::Index,
                                        sizeof(std::uint32_t),
                                        m_indices.size(),
                                        m_indices.data(),
                                        false,
                                        "MeshIndexBuffer");
    }

} // namespace worse