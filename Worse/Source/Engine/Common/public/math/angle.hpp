#pragma once
#include "base_type.hpp"
#include "macro/common_macro.hpp"
#include "math/math_constants.hpp"
#include "math/comparison.hpp"

namespace Worse
{

    WORSE_FORCE_INLINE Float DegreesToRadians(Float const degrees)
    {
        return degrees * kDegreesToRadians;
    }

    WORSE_FORCE_INLINE Float RadiansToDegrees(Float const radians)
    {
        return radians * kRadiansToDegrees;
    }

    class Radian
    {
    public:
        constexpr Radian()
        {
        }
        explicit constexpr Radian(Float const value)
            : m_value{value}
        {
        }

        WORSE_FORCE_INLINE Float AsRadians() const
        {
            return m_value;
        }
        WORSE_FORCE_INLINE Float AsDegrees() const
        {
            return RadiansToDegrees(m_value);
        }

        Radian& operator=(Float const value)
        {
            m_value = value;
            return *this;
        }
        explicit operator Float() const
        {
            return m_value;
        }

        Bool operator==(Radian const& rhs) const
        {
            return Eq(m_value, rhs.m_value);
        }
        Bool operator!=(Radian const& rhs) const
        {
            return Neq(m_value, rhs.m_value);
        }

        Radian operator+() const
        {
            return *this;
        }
        Radian operator-() const
        {
            return Radian{-m_value};
        }

        Radian operator+(Radian const& rhs) const
        {
            return Radian{m_value + rhs.m_value};
        }
        Radian operator-(Radian const& rhs) const
        {
            return Radian{m_value - rhs.m_value};
        }
        Radian operator*(Radian const& rhs) const
        {
            return Radian{m_value * rhs};
        }
        Radian operator/(Radian const& rhs) const
        {
            WORSE_ASSERT(rhs.m_value != 0.0f);
            return Radian{m_value / rhs};
        }

        friend Radian operator*(Float const scalar, Radian const& rhs)
        {
            return Radian{rhs.m_value * scalar};
        }
        friend Radian operator/(Float const scalar, Radian const& rhs)
        {
            WORSE_ASSERT(rhs.m_value != 0.0f);
            return Radian{scalar / rhs.m_value};
        }

        Radian& operator+=(Radian const& rhs)
        {
            m_value += rhs.m_value;
            return *this;
        }
        Radian& operator-=(Radian const& rhs)
        {
            m_value -= rhs.m_value;
            return *this;
        }
        Radian& operator*=(Float const rhs)
        {
            m_value *= rhs;
            return *this;
        }
        Radian& operator/=(Float const rhs)
        {
            WORSE_ASSERT(rhs != 0.0f);
            m_value /= rhs;
            return *this;
        }

    private:
        Float m_value = 0.0f;
    };

    class Degree
    {
    public:
        constexpr Degree()
        {
        }
        explicit constexpr Degree(Float const value)
            : m_value{value}
        {
        }

        WORSE_FORCE_INLINE Float AsDegrees() const
        {
            return m_value;
        }
        WORSE_FORCE_INLINE Float AsRadians() const
        {
            return DegreesToRadians(m_value);
        }

        Degree& operator=(Float const value)
        {
            m_value = value;
            return *this;
        }
        explicit operator Float() const
        {
            return m_value;
        }

        Bool operator==(Degree const& rhs) const
        {
            return Eq(m_value, rhs.m_value);
        }
        Bool operator!=(Degree const& rhs) const
        {
            return Neq(m_value, rhs.m_value);
        }

        Degree operator+() const
        {
            return *this;
        }
        Degree operator-() const
        {
            return Degree{-m_value};
        }

        Degree operator+(Degree const& rhs) const
        {
            return Degree{m_value + rhs.m_value};
        }
        Degree operator-(Degree const& rhs) const
        {
            return Degree{m_value - rhs.m_value};
        }
        Degree operator*(Float const rhs) const
        {
            return Degree{m_value * rhs};
        }
        Degree operator/(Float const rhs) const
        {
            WORSE_ASSERT(rhs != 0.0f);
            return Degree{m_value / rhs};
        }

        friend Degree operator*(Float const scalar, Degree const& rhs)
        {
            return Degree{rhs.m_value * scalar};
        }
        friend Degree operator/(Float const scalar, Degree const& rhs)
        {
            WORSE_ASSERT(rhs.m_value != 0.0f);
            return Degree{scalar / rhs.m_value};
        }

        Degree& operator+=(Degree const& rhs)
        {
            m_value += rhs.m_value;
            return *this;
        }
        Degree& operator-=(Degree const& rhs)
        {
            m_value -= rhs.m_value;
            return *this;
        }
        Degree& operator*=(Float const rhs)
        {
            m_value *= rhs;
            return *this;
        }
        Degree& operator/=(Float const rhs)
        {
            WORSE_ASSERT(rhs != 0.0f);
            m_value /= rhs;
            return *this;
        }

    private:
        Float m_value = 0.0f;
    };

} // namespace Worse