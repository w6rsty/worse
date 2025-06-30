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

    struct Drawcall
    {
        std::uint32_t meshIndex     = 0;
        std::uint32_t materialIndex = 0;
        math::Matrix4 transform;
    };

    // clang-format off
    inline void buildDrawcalls(
        ecs::Commands commands,
        ecs::QueryView<Mesh3D, LocalTransform> view,
        ecs::ResourceArray<Drawcall> drawcalls
    )
    {
        drawcalls.clear();

        view.each(
        [&commands, &drawcalls]
        (ecs::Entity entity, Mesh3D const& mesh, LocalTransform const& transform)
        {
            // TODO: add a default material if not exists
            MeshMaterial material = commands.getComponent<MeshMaterial>(entity);

            std::uint32_t materialIndex = 
            drawcalls->add(
                static_cast<std::uint32_t>(mesh.index),
                static_cast<std::uint32_t>(material.index),
                math::makeSRT(transform.scale,
                              transform.rotation,
                              transform.position)
            );
        });
    }
    // clang-format on

} // namespace worse