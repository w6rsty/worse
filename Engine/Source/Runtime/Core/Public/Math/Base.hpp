#pragma once

#include <limits>
#include <cmath>
#include <cassert>

namespace worse
{

// for performance concerns, we use assert directly instead of logging
#define WS_ASSERT_MATH(condition, message) assert((condition) && (message))

    constexpr float F32MIN     = std::numeric_limits<float>::min();
    constexpr float F32MAX     = std::numeric_limits<float>::max();
    constexpr float F32INF     = std::numeric_limits<float>::infinity();
    constexpr float F32NAN     = std::numeric_limits<float>::quiet_NaN();
    constexpr float F32EPSILON = std::numeric_limits<float>::epsilon();

    constexpr float PI = 3.14159265359f;

    // clang-format off
    constexpr float toRadians(float const degrees) { return degrees * PI / 180.0f; }
    constexpr float toDegrees(float const radians) { return radians * 180.0f / PI; }

    template <typename T>
    concept Arithmetic = std::is_arithmetic_v<T>;

    template <Arithmetic T>
    bool isFinite(T const& value) { return std::isfinite(value); }

    template <Arithmetic T>
    inline bool equal(T const& a, T const& b)
    { 
        return std::abs(a - b) < static_cast<T>(1e-6f);
    }

    template <Arithmetic T>
    inline bool equal(T const& a, T const& b, T const epsilon)
    {
        return std::abs(a - b) < epsilon;
    }

    /// Liner interpolation
    template <Arithmetic T>
    inline constexpr T lerp(T const& a, T const& b, float const t)
    {
        return a + (b - a) * t;
    }

    template <Arithmetic T>
    inline constexpr T clamp(T const& value, T const& min, T const& max)
    {
        return value < min ? min : value > max ? max : value;
    }

    template <Arithmetic T>
    inline constexpr T signum(T const& value)
    {
        return value < 0 ? -1 : value > 0 ? 1 : 0;
    }

    // clang-format on

    /* Forward declaration */
    struct Vector2;
    struct Vector3;
    struct Vector4;
    struct Matrix2;
    struct Matrix3;
    struct Matrix4;
    struct Quaternion;

} // namespace worse