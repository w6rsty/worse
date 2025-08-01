#pragma once
#include "Material.hpp"
#include "Math/Transform.hpp"
#include "Mesh.hpp"
#include "Prefab.hpp"

#include "ECS/Commands.hpp"
#include "ECS/QueryView.hpp"
#include "ECS/Resource.hpp"

namespace worse
{

    struct RenderObject
    {
        Mesh* mesh;
        MeshMaterial* material;

        math::Matrix4 transform;
    };

    struct DrawContext
    {
        std::vector<RenderObject> opaqueObjects;

        void Clear()
        {
            opaqueObjects.clear();
        }
    };

    struct IRenderable
    {
        virtual void Draw(math::Matrix4 const& topMat, DrawContext& ctx) = 0;
    };

    struct Drawcall
    {
        std::weak_ptr<Mesh> mesh;
        u32 materialIndex = 0;
        math::Matrix4 transform;
    };

    struct DrawcallStorage
    {
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
        ecs::QueryView<Mesh3D, LocalTransform> view,
        ecs::Resource<DrawcallStorage> drawcalls
    )
    {
        drawcalls->clear();

        view.each(
        [&commands, &drawcalls]
        (ecs::Entity entity, Mesh3D const& mesh, LocalTransform const& transform)
        {
            // TODO: add a default material if not exists
            MeshMaterial material = commands.getComponent<MeshMaterial>(entity);

            if (mesh.primitiveTopology == RHIPrimitiveTopology::PointList)
            {
                drawcalls->point.emplace_back(
                    mesh.mesh,
                    static_cast<u32>(material.index),
                    math::makeSRT(transform.scale,
                                transform.rotation,
                                transform.position)
                );
            }
            else
            {
                drawcalls->solid.emplace_back(
                    mesh.mesh,
                    static_cast<u32>(material.index),
                    math::makeSRT(transform.scale,
                                transform.rotation,
                                transform.position)
                );
            }
        });
    }
    // clang-format on

} // namespace worse