#pragma once
#include "Math/Math.hpp"

namespace worse
{

    class Camera
    {
    public:
        math::Vector3 position       = {0.0f, 0.0f, 0.0f};
        math::Quaternion orientation = {1.0f, 0.0f, 0.0f, 0.0f};

        float near        = 0.1f;
        float far         = 1000.0f;
        float fovv        = 60.0f;
        float aspectRatio = 1.0f;
    };

} // namespace worse