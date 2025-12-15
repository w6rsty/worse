#pragma once

#include "Macro/Common.hpp"
#include "Math/Math.hpp"
#include "Math/Comparison.hpp"

namespace worse
{

    WORSE_FORCE_INLINE F32 DegreesToRadians(F32 const degrees)
    {
        return degrees * FMath::kDegreesToRadians;
    }

    WORSE_FORCE_INLINE F32 RadiansToDegrees(F32 const radians)
    {
        return radians * FMath::kRadiansToDegrees;
    }

    class Radian
    {
    public:
        constexpr Radian()
        {
        }
        explicit constexpr Radian(F32 const value)
            : m_Value{value}
        {
        }

        WORSE_FORCE_INLINE F32 AsRadians() const
        {
            return m_Value;
        }
        WORSE_FORCE_INLINE F32 AsDegrees() const
        {
            return RadiansToDegrees(m_Value);
        }

        Radian& operator=(F32 const value)
        {
            m_Value = value;
            return *this;
        }
        explicit operator F32() const
        {
            return m_Value;
        }

        bool operator==(Radian const& rhs) const
        {
            return Eq(m_Value, rhs.m_Value);
        }
        bool operator!=(Radian const& rhs) const
        {
            return Neq(m_Value, rhs.m_Value);
        }

        Radian operator+() const
        {
            return *this;
        }
        Radian operator-() const
        {
            return Radian{-m_Value};
        }

        Radian operator+(Radian const& rhs) const
        {
            return Radian{m_Value + rhs.m_Value};
        }
        Radian operator-(Radian const& rhs) const
        {
            return Radian{m_Value - rhs.m_Value};
        }
        Radian operator*(Radian const& rhs) const
        {
            return Radian{m_Value * rhs};
        }
        Radian operator/(Radian const& rhs) const
        {
            WORSE_ASSERT(rhs.m_Value != 0.0f);
            return Radian{m_Value / rhs};
        }

        friend Radian operator*(F32 const scalar, Radian const& rhs)
        {
            return Radian{rhs.m_Value * scalar};
        }
        friend Radian operator/(F32 const scalar, Radian const& rhs)
        {
            WORSE_ASSERT(rhs.m_Value != 0.0f);
            return Radian{scalar / rhs.m_Value};
        }

        Radian& operator+=(Radian const& rhs)
        {
            m_Value += rhs.m_Value;
            return *this;
        }
        Radian& operator-=(Radian const& rhs)
        {
            m_Value -= rhs.m_Value;
            return *this;
        }
        Radian& operator*=(F32 const rhs)
        {
            m_Value *= rhs;
            return *this;
        }
        Radian& operator/=(F32 const rhs)
        {
            WORSE_ASSERT(rhs != 0.0f);
            m_Value /= rhs;
            return *this;
        }

    private:
        F32 m_Value = 0.0f;
    };

    class Degree
    {
    public:
        constexpr Degree()
        {
        }
        explicit constexpr Degree(F32 const value)
            : m_Value{value}
        {
        }

        WORSE_FORCE_INLINE F32 AsDegrees() const
        {
            return m_Value;
        }
        WORSE_FORCE_INLINE F32 AsRadians() const
        {
            return DegreesToRadians(m_Value);
        }

        Degree& operator=(F32 const value)
        {
            m_Value = value;
            return *this;
        }
        explicit operator F32() const
        {
            return m_Value;
        }

        bool operator==(Degree const& rhs) const
        {
            return Eq(m_Value, rhs.m_Value);
        }
        bool operator!=(Degree const& rhs) const
        {
            return Neq(m_Value, rhs.m_Value);
        }

        Degree operator+() const
        {
            return *this;
        }
        Degree operator-() const
        {
            return Degree{-m_Value};
        }

        Degree operator+(Degree const& rhs) const
        {
            return Degree{m_Value + rhs.m_Value};
        }
        Degree operator-(Degree const& rhs) const
        {
            return Degree{m_Value - rhs.m_Value};
        }
        Degree operator*(F32 const rhs) const
        {
            return Degree{m_Value * rhs};
        }
        Degree operator/(F32 const rhs) const
        {
            WORSE_ASSERT(rhs != 0.0f);
            return Degree{m_Value / rhs};
        }

        friend Degree operator*(F32 const scalar, Degree const& rhs)
        {
            return Degree{rhs.m_Value * scalar};
        }
        friend Degree operator/(F32 const scalar, Degree const& rhs)
        {
            WORSE_ASSERT(rhs.m_Value != 0.0f);
            return Degree{scalar / rhs.m_Value};
        }

        Degree& operator+=(Degree const& rhs)
        {
            m_Value += rhs.m_Value;
            return *this;
        }
        Degree& operator-=(Degree const& rhs)
        {
            m_Value -= rhs.m_Value;
            return *this;
        }
        Degree& operator*=(F32 const rhs)
        {
            m_Value *= rhs;
            return *this;
        }
        Degree& operator/=(F32 const rhs)
        {
            WORSE_ASSERT(rhs != 0.0f);
            m_Value /= rhs;
            return *this;
        }

    private:
        F32 m_Value = 0.0f;
    };

} // namespace worse