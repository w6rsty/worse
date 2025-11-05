#pragma once
#include "math/math_includes.hpp"
#include "ECS/Entity.hpp"

#include <string>
#include <vector>

namespace Worse
{
    struct GlobalContext
    {
        Float deltaTime      = 0.0f;
        Float time           = 0.0f;
        Bool isWireFrameMode = false;
    };

    struct Object
    {
        ULong id;
        std::string name;
    };

    struct Parent
    {
        ecs::Entity parent;
    };

    struct Children
    {
        std::vector<ecs::Entity> children;
    };

    struct LocalTransform
    {
        Vector3 position          = Vector3::ZERO;
        math::Quaternion rotation = math::Quaternion::IDENTITY();
        Vector3 scale             = Vector3::ONE;
    };

} // namespace Worse