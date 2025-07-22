#pragma once
#include "Math/BoundingBox.hpp"
#include "Geometry/GeometryGeneration.hpp"
#include "RHIBuffer.hpp"
#include "RHIVertex.hpp"

#include <concepts>
#include <memory>

namespace worse
{
    class Mesh;

    struct Mesh3D
    {
        std::shared_ptr<Mesh> mesh;
        RHIPrimitiveTopology primitiveTopology =
            RHIPrimitiveTopology::TriangleList;
    };

    struct StandardMesh3D
    {
        geometry::GeometryType type;
    };

    struct CustomMesh3D : StandardMesh3D
    {
        RHIVertexType vertexType;
        std::span<std::byte> vertices;
        std::optional<std::vector<std::uint32_t>> indices = std::nullopt;
    };

    struct Quad3D : StandardMesh3D
    {
        float width  = 1.0f;
        float height = 1.0f;
    };

    struct Cube : StandardMesh3D
    {
        float width  = 1.0f;
        float height = 1.0f;
        float depth  = 1.0f;
    };

    struct Sphere : StandardMesh3D
    {
        float radius           = 0.5f;
        std::uint32_t segments = 32; // horizontal segments
        std::uint32_t rings    = 16; // vertical segments
    };

    struct Cylinder : StandardMesh3D
    {
        float radius           = 0.5f;
        float height           = 1.0f;
        std::uint32_t segments = 32; // horizontal segments
    };

    struct Capsule3D : StandardMesh3D
    {
        float radius           = 0.5f;
        float height           = 2.0f;
        std::uint32_t segments = 32; // horizontal segments
        std::uint32_t rings    = 16; // vertical segments
    };

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

        template <std::derived_from<StandardMesh3D> T>
        Mesh(T const& standardMesh)
        {
            if constexpr (std::is_same_v<T, CustomMesh3D>)
            {
                addGeometry(
                    standardMesh.vertexType,
                    standardMesh.vertices,
                    standardMesh.indices.value_or(std::vector<std::uint32_t>{}));
                return;
            }

            std::vector<std::byte> vertices;
            std::vector<std::uint32_t> indices;

            if constexpr (std::is_same_v<T, Quad3D>)
            {
                geometry::generateQuad3D(vertices,
                                         indices,
                                         standardMesh.width,
                                         standardMesh.height);
            }
            else if constexpr (std::is_same_v<T, Cube>)
            {
                geometry::generateCube(vertices,
                                       indices,
                                       standardMesh.width,
                                       standardMesh.height,
                                       standardMesh.depth);
            }
            else if constexpr (std::is_same_v<T, Sphere>)
            {
                geometry::generateSphere(vertices,
                                         indices,
                                         standardMesh.radius,
                                         standardMesh.segments,
                                         standardMesh.rings);
            }
            else if constexpr (std::is_same_v<T, Cylinder>)
            {
                geometry::generateCylinder(vertices,
                                           indices,
                                           standardMesh.radius,
                                           standardMesh.height,
                                           standardMesh.segments);
            }
            else if constexpr (std::is_same_v<T, Capsule3D>)
            {
                geometry::generateCapsule(vertices,
                                          indices,
                                          standardMesh.radius,
                                          standardMesh.height,
                                          standardMesh.segments,
                                          standardMesh.rings);
            }

            addGeometry(RHIVertexType::PosUvNrmTan, vertices, indices);
        }

        ~Mesh();

        void clearCPU();
        void clearGPU();
        void clearAll();

        // TODO: Now just lod0
        void addGeometry(RHIVertexType vertexType, std::span<std::byte> vertices,
                         std::vector<std::uint32_t> const& indices);

        void createGPUBuffers();

        void setVisible(bool visible)
        {
            m_visible = visible;
        }

        // clang-format off
        bool isVisible() const { return m_visible; }
        RHIBuffer* getVertexBuffer() const { return m_vertexBuffer.get(); }
        RHIBuffer* getIndexBuffer() const { return m_indexBuffer.get(); }
        // clang-format on

    private:
        bool m_visible = true;
        RHIVertexType m_vertexType;
        std::vector<std::byte> m_vertices;
        std::vector<std::uint32_t> m_indices;
        std::vector<SubMesh> m_subMeshes;

        std::shared_ptr<RHIBuffer> m_vertexBuffer = nullptr;
        std::shared_ptr<RHIBuffer> m_indexBuffer  = nullptr;
    };

} // namespace worse