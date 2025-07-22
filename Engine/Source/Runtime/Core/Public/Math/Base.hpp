#pragma once
#include "Types.hpp"

#include <limits>
#include <cmath>
#include <cassert>

namespace worse::math
{

// for performance concerns, we use assert directly instead of logging
#define WS_ASSERT_MATH(condition, message) assert((condition) && (message))

    constexpr f32 F32MIN     = std::numeric_limits<f32>::min();
    constexpr f32 F32MAX     = std::numeric_limits<f32>::max();
    constexpr f32 F32INF     = std::numeric_limits<f32>::infinity();
    constexpr f32 F32NAN     = std::numeric_limits<f32>::quiet_NaN();
    constexpr f32 F32EPSILON = std::numeric_limits<f32>::epsilon();

    constexpr f32 PI = 3.14159265359f;

    // clang-format off
    constexpr f32 toRadians(f32 const degrees) { return degrees * PI / 180.0f; }
    constexpr f32 toDegrees(f32 const radians) { return radians * 180.0f / PI; }

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
    inline constexpr T lerp(T const& a, T const& b, f32 const t)
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

} // namespace worse::math