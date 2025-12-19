#pragma once

#include "BaseTypes.hpp"
#include "Macro/Common.hpp"

#include <cmath>
#include <limits>

namespace worse
{

    struct FMath
    {
        //
        // Constants
        //

        inline static constexpr F32 kFloatMax         = std::numeric_limits<F32>::max();
        inline static constexpr F32 kFloatMin         = -std::numeric_limits<F32>::max();
        inline static constexpr F32 kFloatInf         = std::numeric_limits<F32>::infinity();
        inline static constexpr F32 kFloatInfNegative = -std::numeric_limits<F32>::infinity();

        // no invalid operation
        inline static constexpr F32 kFloatNaN = std::numeric_limits<F32>::quiet_NaN();

        inline static constexpr F32 kFloatEpsilon = 1e-6f;

        inline static constexpr F32 kPi    = 3.14159265359f;
        inline static constexpr F32 kEuler = 2.71828182846f;

        inline static constexpr F32 kDegreesToRadians = kPi / 180.0f;
        inline static constexpr F32 kRadiansToDegrees = 180.0f / kPi;

        //
        // Functions
        //

        WORSE_FORCE_INLINE static F32 Signum(F32 const& value)
        {
            return value < 0 ? -1 : value > 0 ? 1
                                              : 0;
        }

        WORSE_FORCE_INLINE static F32 Square(F32 const x)
        {
            return x * x;
        }

        WORSE_FORCE_INLINE static F32 Sqrt(F32 const x)
        {
            return std::sqrt(x);
        }

        WORSE_FORCE_INLINE static F32 Cbrt(F32 const x)
        {
            return std::cbrt(x);
        }

        WORSE_FORCE_INLINE static F32 Pow(F32 const base, F32 const exp)
        {
            return std::pow(base, exp);
        }

        WORSE_FORCE_INLINE static F32 Exp(F32 const x)
        {
            return std::exp(x);
        }

        WORSE_FORCE_INLINE static F32 Log(F32 const x)
        {
            return std::log(x);
        }

        WORSE_FORCE_INLINE static F32 Log2(F32 const x)
        {
            return std::log2(x);
        }

        WORSE_FORCE_INLINE static F32 Log10(F32 const x)
        {
            return std::log10(x);
        }

        WORSE_FORCE_INLINE static F32 Abs(F32 const x)
        {
            return std::fabs(x);
        }

        template <typename T>
        WORSE_FORCE_INLINE static T Clamp(T const x, T const min, T const max)
        {
            return (x < min) ? min : (x > max) ? max
                                               : x;
        }

        WORSE_FORCE_INLINE static F32 Saturate(F32 const x)
        {
            return FMath::Clamp<F32>(x, 0.0f, 1.0f);
        }

        WORSE_FORCE_INLINE static F32 Lerp(F32 const a, F32 const b, F32 const t)
        {
            return a + (b - a) * t;
        }

        WORSE_FORCE_INLINE static F32 Sin(F32 const radians)
        {
            return std::sin(radians);
        }

        WORSE_FORCE_INLINE static F32 Cos(F32 const radians)
        {
            return std::cos(radians);
        }

        WORSE_FORCE_INLINE static F32 Tan(F32 const radians)
        {
            return std::tan(radians);
        }

        WORSE_FORCE_INLINE static F32 Asin(F32 const value)
        {
            return std::asin(value);
        }

        WORSE_FORCE_INLINE static F32 Acos(F32 const value)
        {
            return std::acos(value);
        }

        WORSE_FORCE_INLINE static F32 Atan(F32 const value)
        {
            return std::atan(value);
        }

        WORSE_FORCE_INLINE static F32 Atan2(F32 const y, F32 const x)
        {
            return std::atan2(y, x);
        }
    };

} // namespace worse