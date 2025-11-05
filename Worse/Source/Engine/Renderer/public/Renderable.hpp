#pragma once
#include "math/transform.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Prefab.hpp"

#include "ECS/Commands.hpp"
#include "ECS/QueryView.hpp"
#include "ECS/Resource.hpp"

namespace Worse
{

    struct RenderObject
    {
        UInt indexCount;
        UInt startIndex;

        Mesh* mesh;
        AssetHandle material;

        Matrix4 transform;
    };

    struct DrawContext
    {
        std::vector<RenderObject> opaqueObjects;

        void clear()
        {
            opaqueObjects.clear();
        }
    };

    struct IRenderable
    {
        virtual void draw(Matrix4 const& topMat, DrawContext& ctx) = 0;
    };

    struct Drawcall
    {
        Mesh* mesh;
        UInt materialIndex = 0;
        Matrix4 transform;
    };

    struct DrawcallStorage
    {
        // TODO: temporary hack
        DrawContext ctx;

        std::vector<Drawcall> solid;
        std::vector<Drawcall> wireframe;
        std::vector<Drawcall> point;

        void clear()
        {
            solid.clear();
            wireframe.clear();
            point.clear();
        }
    };

    // clang-format off
    inline void buildDrawcalls(
        ecs::Commands commands,
        ecs::QueryView<Mesh3D, LocalTransform, MeshMaterial> view,
        ecs::Resource<DrawcallStorage> drawcalls
    )
    {
        drawcalls->clear();

        view.each(
        [&commands, &drawcalls]
        (ecs::Entity entity, Mesh3D const& mesh, LocalTransform const& transform, MeshMaterial const& material)
        {
            if (mesh.primitiveTopology == RHIPrimitiveTopology::PointList)
            {
                drawcalls->point.emplace_back(
                    mesh.mesh,
                    static_cast<UInt>(material.index),
                    math::makeSRT(transform.scale, transform.rotation, transform.position)
                );
            }
            else
            {
                drawcalls->solid.emplace_back(
                    mesh.mesh,
                    static_cast<UInt>(material.index),
                    math::makeSRT(transform.scale, transform.rotation, transform.position)
                );
            }
        });
    }
    // clang-format on

} // namespace Worse