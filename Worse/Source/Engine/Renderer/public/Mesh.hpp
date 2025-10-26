#pragma once
#include "Math/BoundingBox.hpp"
#include "Geometry/GeometryGeneration.hpp"
#include "RHIBuffer.hpp"

#include "ECS/Resource.hpp"
#include "ECS/QueryView.hpp"

#include <concepts>
#include <memory>

namespace worse
{

    class Mesh;

    // ECS index
    struct Mesh3D
    {
        Mesh* mesh;
        RHIPrimitiveTopology primitiveTopology = RHIPrimitiveTopology::TriangleList;
    };

    struct StandardMesh3D
    {
        geometry::GeometryType type;
    };

    struct CustomMesh3D : StandardMesh3D
    {
        std::vector<RHIVertexPosUvNrmTan> vertices;
        std::optional<std::vector<u32>> indices = std::nullopt;
    };

    struct Quad3D : StandardMesh3D
    {
        f32 width  = 1.0f;
        f32 height = 1.0f;
    };

    struct Cube : StandardMesh3D
    {
        f32 width  = 1.0f;
        f32 height = 1.0f;
        f32 depth  = 1.0f;
    };

    struct Sphere : StandardMesh3D
    {
        f32 radius   = 0.5f;
        u32 segments = 32; // horizontal segments
        u32 rings    = 16; // vertical segments
    };

    struct Cylinder : StandardMesh3D
    {
        f32 radius   = 0.5f;
        f32 height   = 1.0f;
        u32 segments = 32; // horizontal segments
    };

    struct Capsule3D : StandardMesh3D
    {
        f32 radius   = 0.5f;
        f32 height   = 2.0f;
        u32 segments = 32; // horizontal segments
        u32 rings    = 16; // vertical segments
    };

    struct MeshLod
    {
        u32 vertexCount;
        u32 vertexOffset;
        u32 indexCount;
        u32 indexOffset;

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
                addGeometry(standardMesh.vertices,
                            standardMesh.indices.value_or(std::vector<u32>{}));
                return;
            }

            std::vector<RHIVertexPosUvNrmTan> vertices;
            std::vector<u32> indices;

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

            addGeometry(vertices, indices);
        }

        ~Mesh();

        void clearCPU();
        void clearGPU();

        // TODO: Now just lod0
        void addGeometry(std::vector<RHIVertexPosUvNrmTan> const& vertices,
                         std::vector<u32> const& indices);

        void createGPUBuffers();

        // clang-format off
        RHIBuffer* getVertexBuffer() const { return m_vertexBuffer.get(); }
        RHIBuffer* getIndexBuffer() const { return m_indexBuffer.get(); }
        // clang-format on

    private:
        std::vector<RHIVertexPosUvNrmTan> m_vertices;
        std::vector<u32> m_indices;
        std::vector<SubMesh> m_subMeshes;

        std::shared_ptr<RHIBuffer> m_vertexBuffer = nullptr;
        std::shared_ptr<RHIBuffer> m_indexBuffer  = nullptr;
    };

    // clang-format off
    void buildMeshes(
        ecs::QueryView<Mesh3D> view,
        ecs::ResourceArray<Mesh> meshes
    );
    // clang-format on

} // namespace worse