#pragma once
#include "base_type.hpp"

#include <limits>

namespace Worse
{

    inline constexpr Float kFloatMax         = std::numeric_limits<Float>::max();
    inline constexpr Float kFloatMin         = -std::numeric_limits<Float>::max();
    inline constexpr Float kFloatInf         = std::numeric_limits<Float>::infinity();
    inline constexpr Float kFloatInfNegative = -std::numeric_limits<Float>::infinity();

    // no invalid operation
    inline constexpr Float kFloatNaN = std::numeric_limits<Float>::quiet_NaN();

    inline constexpr Float kFloatEpsilon = 1e-6f;

    inline constexpr Float kPi    = 3.14159265359f;
    inline constexpr Float kEuler = 2.71828182846f;

    inline constexpr Float kDegreesToRadians = kPi / 180.0f;
    inline constexpr Float kRadiansToDegrees = 180.0f / kPi;

} // namespace Worse