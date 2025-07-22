#pragma once
#include "Material.hpp"
#include "Math/Transform.hpp"
#include "Mesh.hpp"
#include "Prefab.hpp"

#include "ECS/Commands.hpp"
#include "ECS/QueryView.hpp"
#include "ECS/Resource.hpp"

#include <memory>

namespace worse
{

    struct Drawcall
    {
        std::weak_ptr<Mesh> mesh;
        std::uint32_t materialIndex = 0;
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
                    static_cast<std::uint32_t>(material.index),
                    math::makeSRT(transform.scale,
                                transform.rotation,
                                transform.position)
                );
            }
            else
            {
                drawcalls->solid.emplace_back(
                    mesh.mesh,
                    static_cast<std::uint32_t>(material.index),
                    math::makeSRT(transform.scale,
                                transform.rotation,
                                transform.position)
                );
            }
        });
    }
    // clang-format on

} // namespace worse