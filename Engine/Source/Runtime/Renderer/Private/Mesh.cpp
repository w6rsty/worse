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
        m_vertices.shrink_to_fit();
        m_indices.clear();
        m_indices.shrink_to_fit();
        m_subMeshes.clear();
    }

    void Mesh::addGeometry(RHIVertexType vertexType, std::span<std::byte> vertices,
                           std::vector<std::uint32_t> const& indices)
    {
        m_vertexType = vertexType;

        SubMesh subMesh;

        MeshLod lod0;
        std::uint32_t vertexCount = vertices.size() / RHIVertexSize(vertexType);
        lod0.vertexCount          = vertexCount;
        lod0.vertexOffset         = vertexCount;
        lod0.indexCount           = static_cast<std::uint32_t>(indices.size());
        lod0.indexOffset          = static_cast<std::uint32_t>(m_indices.size());

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

        m_vertexBuffer = std::make_shared<RHIBuffer>(
            RHIBufferUsageFlagBits::Vertex,
            RHIVertexSize(m_vertexType),
            m_vertices.size() / RHIVertexSize(m_vertexType),
            m_vertices.data(),
            false,
            "MeshVertexBuffer");

        if (m_indices.size() != 0)
        {
            m_indexBuffer =
                std::make_shared<RHIBuffer>(RHIBufferUsageFlagBits::Index,
                                            sizeof(std::uint32_t),
                                            m_indices.size(),
                                            m_indices.data(),
                                            false,
                                            "MeshIndexBuffer");
        }
    }

    // clang-format off
    void buildMeshes(
        ecs::QueryView<Mesh3D> view,
        ecs::ResourceArray<Mesh> meshes
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