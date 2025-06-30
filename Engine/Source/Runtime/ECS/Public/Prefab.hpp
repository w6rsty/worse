#pragma once
#include "Math/Math.hpp"
#include "ECS/Entity.hpp"

#include <string>
#include <vector>

namespace worse
{
    struct GlobalContext
    {
        float deltaTime      = 0.0f;
        float time           = 0.0f;
        bool isWireFrameMode = false;
    };

    struct Object
    {
        std::uint64_t id;
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
        math::Vector3 position    = math::Vector3::ZERO();
        math::Quaternion rotation = math::Quaternion::IDENTITY();
        math::Vector3 scale       = math::Vector3::ONE();
    };

} // namespace worse