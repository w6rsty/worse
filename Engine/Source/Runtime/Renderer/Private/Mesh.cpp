#include "Mesh.hpp"
#include "RHIBuffer.hpp"
#include "Log.hpp"

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
                           std::vector<u32> const& indices)
    {
        SubMesh subMesh;

        MeshLod lod0;
        lod0.vertexCount  = static_cast<u32>(vertices.size());
        lod0.vertexOffset = static_cast<u32>(m_vertices.size());
        lod0.indexCount   = static_cast<u32>(indices.size());
        lod0.indexOffset  = static_cast<u32>(m_indices.size());
        lod0.boundingBox  = math::BoundingBox(vertices);

        subMesh.lods.push_back(lod0);

        m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
        m_indices.insert(m_indices.end(), indices.begin(), indices.end());

        m_subMeshes.push_back(subMesh);
    }

    void Mesh::createGPUBuffers()
    {
        if (m_vertices.empty())
        {
            WS_LOG_WARN("Mesh", "No vertices");
            return;
        }

        m_vertexBuffer = std::make_shared<RHIBuffer>(RHIBufferUsageFlagBits::Vertex,
                                                     sizeof(RHIVertexPosUvNrmTan),
                                                     m_vertices.size(),
                                                     m_vertices.data(),
                                                     false,
                                                     "MeshVertexBuffer");

        if (m_indices.size() != 0)
        {
            m_indexBuffer = std::make_shared<RHIBuffer>(RHIBufferUsageFlagBits::Index,
                                                        sizeof(u32),
                                                        m_indices.size(),
                                                        m_indices.data(),
                                                        false,
                                                        "MeshIndexBuffer");
        }
    }

    // clang-format off
    void buildMeshes( ecs::QueryView<Mesh3D> view, ecs::ResourceArray<Mesh> meshes
    )
    {
        view.each(
        [&meshes]
        (ecs::Entity entity, Mesh3D const& mesh3D)
        {
            meshes->get(mesh3D.index)->createGPUBuffers();
        });
    }
    // clang-format on
} // namespace worse