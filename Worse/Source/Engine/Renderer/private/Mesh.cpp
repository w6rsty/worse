#include "Mesh.hpp"
#include "RHIBuffer.hpp"
#include "logger/logger.hpp"

namespace Worse
{

    Mesh::Mesh()
    {
    }

    Mesh::~Mesh()
    {
        // CPU 和 GPU 资源会自动释放
    }

    void Mesh::clearCPU()
    {
        m_vertices.clear();
        m_indices.clear();
        m_subMeshes.clear();
    }

    void Mesh::clearGPU()
    {
        m_vertexBuffer.reset();
        m_indexBuffer.reset();
    }

    void Mesh::addGeometry(std::vector<RHIVertexPosUvNrmTan> const& vertices, std::vector<UInt> const& indices)
    {
        SubMesh subMesh;

        MeshLod lod0;
        lod0.vertexCount  = static_cast<UInt>(vertices.size());
        lod0.vertexOffset = static_cast<UInt>(m_vertices.size());
        lod0.indexCount   = static_cast<UInt>(indices.size());
        lod0.indexOffset  = static_cast<UInt>(m_indices.size());

        subMesh.lods.push_back(lod0);

        m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
        m_indices.insert(m_indices.end(), indices.begin(), indices.end());

        m_subMeshes.push_back(subMesh);
    }

    void Mesh::createGPUBuffers()
    {
        if (m_vertices.empty())
        {
            WORSE_LOG_WARN("Mesh", "No vertices");
            return;
        }

        m_vertexBuffer = std::make_shared<RHIBuffer>(
            RHIBufferUsage::FlagBits::Vertex,
            sizeof(RHIVertexPosUvNrmTan),
            m_vertices.size(),
            m_vertices.data(),
            false,
            "MeshVertexBuffer");

        if (m_indices.size() != 0)
        {
            m_indexBuffer = std::make_shared<RHIBuffer>(
                RHIBufferUsage::FlagBits::Index,
                sizeof(UInt),
                m_indices.size(),
                m_indices.data(),
                false,
                "MeshIndexBuffer");
        }
    }
} // namespace Worse