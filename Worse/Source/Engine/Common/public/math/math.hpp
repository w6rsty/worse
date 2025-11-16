#pragma once
#include "base_type.hpp"
#include "macro/common_macro.hpp"
#include "math/comparison.hpp"

namespace Worse
{

    class Math
    {
    public:
        WORSE_FORCE_INLINE static Float Signum(Float const& value)
        {
            return value < 0 ? -1 : value > 0 ? 1
                                              : 0;
        }

        WORSE_FORCE_INLINE static Float Square(Float const x)
        {
            return x * x;
        }

        WORSE_FORCE_INLINE static Float Sqrt(Float const x)
        {
            return std::sqrt(x);
        }

        WORSE_FORCE_INLINE static Float Cbrt(Float const x)
        {
            return std::cbrt(x);
        }

        WORSE_FORCE_INLINE static Float Pow(Float const base, Float const exp)
        {
            return std::pow(base, exp);
        }

        WORSE_FORCE_INLINE static Float Exp(Float const x)
        {
            return std::exp(x);
        }

        WORSE_FORCE_INLINE static Float Log(Float const x)
        {
            return std::log(x);
        }

        WORSE_FORCE_INLINE static Float Log2(Float const x)
        {
            return std::log2(x);
        }

        WORSE_FORCE_INLINE static Float Log10(Float const x)
        {
            return std::log10(x);
        }

        WORSE_FORCE_INLINE static Float Abs(Float const x)
        {
            return std::fabs(x);
        }

        template <typename T, typename = std::enable_if_t<Comparison::IsTotallyOrdered_V<std::decay_t<T>>>>
        WORSE_FORCE_INLINE static T Clamp(T const x, T const min, T const max)
        {
            return (x < min) ? min : (x > max) ? max
                                               : x;
        }

        WORSE_FORCE_INLINE static Float Saturate(Float const x)
        {
            return Math::Clamp<Float>(x, 0.0f, 1.0f);
        }

        WORSE_FORCE_INLINE static Float Lerp(Float const a, Float const b, Float const t)
        {
            return a + (b - a) * t;
        }

        WORSE_FORCE_INLINE static Float Sin(Float const radians)
        {
            return std::sin(radians);
        }

        WORSE_FORCE_INLINE static Float Cos(Float const radians)
        {
            return std::cos(radians);
        }

        WORSE_FORCE_INLINE static Float Tan(Float const radians)
        {
            return std::tan(radians);
        }

        WORSE_FORCE_INLINE static Float Asin(Float const value)
        {
            return std::asin(value);
        }

        WORSE_FORCE_INLINE static Float Acos(Float const value)
        {
            return std::acos(value);
        }

        WORSE_FORCE_INLINE static Float Atan(Float const value)
        {
            return std::atan(value);
        }

        WORSE_FORCE_INLINE static Float Atan2(Float const y, Float const x)
        {
            return std::atan2(y, x);
        }
    };

} // namespace Worse