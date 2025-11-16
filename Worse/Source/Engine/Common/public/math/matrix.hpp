#pragma once
#include "base_type.hpp"
#include "macro/common_macro.hpp"
#include "math/math.hpp"
#include "math/vector.hpp"

#include <cstring>

namespace Worse
{

    class Matrix2
    {
    public:
        union
        {
            struct
            {
                Float m00, m10;
                Float m01, m11;
            };
            Float data[4];
        };

    public:
        constexpr Matrix2()
            : m00{0.0f}, m10{0.0f}, m01{0.0f}, m11{0.0f}
        {
        }

        constexpr Matrix2(Float const m00, Float const m01,
                          Float const m10, Float const m11)
            : m00{m00}, m01{m01}, m10{m10}, m11{m11}
        {
        }

        constexpr Matrix2(Vector2 const& col0, Vector2 const& col1)
            : m00{col0.x}, m10{col0.y}, m01{col1.x}, m11{col1.y}
        {
        }

        Float* ptr()
        {
            return &m00;
        }
        Float const* ptr() const
        {
            return &m00;
        }

        Vector2& operator[](Size const index)
        {
            WORSE_ASSERT((index >= 0) && (index < 2));
            return reinterpret_cast<Vector2*>(ptr())[index];
        }
        Vector2 const& operator[](Size const index) const
        {
            WORSE_ASSERT((index >= 0) && (index < 2));
            return reinterpret_cast<const Vector2*>(ptr())[index];
        }

        Bool operator==(Matrix2 const& other) const
        {
            return Eq(m00, other.m00) && Eq(m10, other.m10) &&
                   Eq(m01, other.m01) && Eq(m11, other.m11);
        }
        Bool operator!=(Matrix2 const& other) const
        {
            return Neq(m00, other.m00) || Neq(m10, other.m10) ||
                   Neq(m01, other.m01) || Neq(m11, other.m11);
        }

        Matrix2 operator+() const
        {
            return *this;
        }
        Matrix2 operator-() const
        {
            return Matrix2{-m00, -m01, -m10, -m11};
        }

        Matrix2 operator+(Float const rhs) const
        {
            return Matrix2{m00 + rhs, m01 + rhs, m10 + rhs, m11 + rhs};
        }
        Matrix2 operator+(Matrix2 const& rhs) const
        {
            return Matrix2{m00 + rhs.m00, m01 + rhs.m01, m10 + rhs.m10, m11 + rhs.m11};
        }
        Matrix2 operator-(Float const rhs) const
        {
            return Matrix2{m00 - rhs, m01 - rhs, m10 - rhs, m11 - rhs};
        }
        Matrix2 operator-(Matrix2 const& rhs) const
        {
            return Matrix2{m00 - rhs.m00, m01 - rhs.m01, m10 - rhs.m10, m11 - rhs.m11};
        }
        Matrix2 operator*(Float const rhs) const
        {
            return Matrix2{m00 * rhs, m01 * rhs, m10 * rhs, m11 * rhs};
        }
        Matrix2 operator*(Matrix2 const& rhs) const
        {
            return Matrix2{m00 * rhs.m00 + m01 * rhs.m10, m00 * rhs.m01 + m01 * rhs.m11, m10 * rhs.m00 + m11 * rhs.m10, m10 * rhs.m01 + m11 * rhs.m11};
        }
        Matrix2 operator/(Float const rhs) const
        {
            WORSE_ASSERT(rhs != 0.0f);
            Float const inv = 1.0f / rhs;
            return Matrix2{m00 * inv, m01 * inv, m10 * inv, m11 * inv};
        }

        Matrix2& operator+=(Float const rhs)
        {
            m00 += rhs;
            m01 += rhs;
            m10 += rhs;
            m11 += rhs;
            return *this;
        }
        Matrix2& operator+=(Matrix2 const& rhs)
        {
            m00 += rhs.m00;
            m01 += rhs.m01;
            m10 += rhs.m10;
            m11 += rhs.m11;
            return *this;
        }
        Matrix2& operator-=(Float const rhs)
        {
            m00 -= rhs;
            m01 -= rhs;
            m10 -= rhs;
            m11 -= rhs;
            return *this;
        }
        Matrix2& operator-=(Matrix2 const& rhs)
        {
            m00 -= rhs.m00;
            m01 -= rhs.m01;
            m10 -= rhs.m10;
            m11 -= rhs.m11;
            return *this;
        }
        Matrix2& operator*=(Float const rhs)
        {
            m00 *= rhs;
            m01 *= rhs;
            m10 *= rhs;
            m11 *= rhs;
            return *this;
        }
        Matrix2& operator*=(Matrix2 const& rhs)
        {
            Float const t00 = m00 * rhs.m00 + m01 * rhs.m10;
            Float const t01 = m00 * rhs.m01 + m01 * rhs.m11;
            Float const t10 = m10 * rhs.m00 + m11 * rhs.m10;
            Float const t11 = m10 * rhs.m01 + m11 * rhs.m11;
            m00             = t00;
            m01             = t01;
            m10             = t10;
            m11             = t11;
            return *this;
        }
        Matrix2& operator/=(Float const rhs)
        {
            WORSE_ASSERT(rhs != 0.0f);
            Float const inv = 1.0f / rhs;
            m00 *= inv;
            m01 *= inv;
            m10 *= inv;
            m11 *= inv;
            return *this;
        }

        Vector2 operator*(Vector2 const& v) const
        {
            return Vector2{m00 * v.x + m01 * v.y,
                           m10 * v.x + m11 * v.y};
        }

        WORSE_FORCE_INLINE friend Float Det(Matrix2 const& m)
        {
            return m.m00 * m.m11 - m.m10 * m.m01;
        }

        WORSE_FORCE_INLINE friend Matrix2 Inverse(Matrix2 const& m)
        {
            Float const d = Det(m);
            if (Eq(d, 0.0f))
            {
                return NANM;
            }
            Float const inv = 1.0f / d;
            return Matrix2(m.m11 * inv, -m.m01 * inv, -m.m10 * inv, m.m00 * inv);
        }

        WORSE_FORCE_INLINE friend Matrix2 Transpose(Matrix2 const& m)
        {
            return Matrix2{m.m00, m.m10, m.m01, m.m11};
        }

        static Matrix2 const IDENTITY;
        static Matrix2 const ZERO;
        static Matrix2 const NANM;
    };

    class Matrix3
    {
    public:
        union
        {
            struct
            {
                Float m00, m10, m20;
                Float m01, m11, m21;
                Float m02, m12, m22;
            };
            Float data[9];
        };

    public:
        constexpr Matrix3()
            : m00{0.0f}, m10{0.0f}, m20{0.0f}, m01{0.0f}, m11{0.0f}, m21{0.0f}, m02{0.0f}, m12{0.0f}, m22{0.0f}
        {
        }

        Matrix3(Float const m00, Float const m01, Float const m02,
                Float const m10, Float const m11, Float const m12,
                Float const m20, Float const m21, Float const m22)
            : m00{m00}, m01{m01}, m02{m02}, m10{m10}, m11{m11}, m12{m12}, m20{m20}, m21{m21}, m22{m22}
        {
        }

        constexpr Matrix3(Vector3 const& col0, Vector3 const& col1, Vector3 const& col2)
            : m00{col0.x}, m10{col0.y}, m20{col0.z}, m01{col1.x}, m11{col1.y}, m21{col1.z}, m02{col2.x}, m12{col2.y}, m22{col2.z}
        {
        }

        Float* ptr()
        {
            return &m00;
        }
        Float const* ptr() const
        {
            return &m00;
        }

        Vector3& operator[](Size const index)
        {
            WORSE_ASSERT((index >= 0) && (index < 3));
            return reinterpret_cast<Vector3*>(ptr())[index];
        }
        Vector3 const& operator[](Size const index) const
        {
            WORSE_ASSERT((index >= 0) && (index < 3));
            return reinterpret_cast<const Vector3*>(ptr())[index];
        }

        Bool operator==(Matrix3 const& other) const
        {
            return Eq(m00, other.m00) && Eq(m10, other.m10) && Eq(m20, other.m20) &&
                   Eq(m01, other.m01) && Eq(m11, other.m11) && Eq(m21, other.m21) &&
                   Eq(m02, other.m02) && Eq(m12, other.m12) && Eq(m22, other.m22);
        }
        Bool operator!=(Matrix3 const& other) const
        {
            return Neq(m00, other.m00) || Neq(m10, other.m10) || Neq(m20, other.m20) ||
                   Neq(m01, other.m01) || Neq(m11, other.m11) || Neq(m21, other.m21) ||
                   Neq(m02, other.m02) || Neq(m12, other.m12) || Neq(m22, other.m22);
        }

        Matrix3 operator+() const
        {
            return *this;
        }
        Matrix3 operator-() const
        {
            return Matrix3{-m00, -m01, -m02, -m10, -m11, -m12, -m20, -m21, -m22};
        }

        Matrix3 operator+(Float const scalar) const
        {
            return Matrix3{m00 + scalar, m01 + scalar, m02 + scalar, m10 + scalar, m11 + scalar, m12 + scalar, m20 + scalar, m21 + scalar, m22 + scalar};
        }
        Matrix3 operator+(Matrix3 const& rhs) const
        {
            return Matrix3{m00 + rhs.m00, m01 + rhs.m01, m02 + rhs.m02, m10 + rhs.m10, m11 + rhs.m11, m12 + rhs.m12, m20 + rhs.m20, m21 + rhs.m21, m22 + rhs.m22};
        }
        Matrix3 operator-(Float const scalar) const
        {
            return Matrix3{m00 - scalar, m01 - scalar, m02 - scalar, m10 - scalar, m11 - scalar, m12 - scalar, m20 - scalar, m21 - scalar, m22 - scalar};
        }
        Matrix3 operator-(Matrix3 const& rhs) const
        {
            return Matrix3{m00 - rhs.m00, m01 - rhs.m01, m02 - rhs.m02, m10 - rhs.m10, m11 - rhs.m11, m12 - rhs.m12, m20 - rhs.m20, m21 - rhs.m21, m22 - rhs.m22};
        }
        Matrix3 operator*(Float const scalar) const
        {
            return Matrix3{m00 * scalar, m01 * scalar, m02 * scalar, m10 * scalar, m11 * scalar, m12 * scalar, m20 * scalar, m21 * scalar, m22 * scalar};
        }
        Matrix3 operator*(Matrix3 const& rhs) const
        {
            return Matrix3{
                m00 * rhs.m00 + m01 * rhs.m10 + m02 * rhs.m20,
                m00 * rhs.m01 + m01 * rhs.m11 + m02 * rhs.m21,
                m00 * rhs.m02 + m01 * rhs.m12 + m02 * rhs.m22,
                m10 * rhs.m00 + m11 * rhs.m10 + m12 * rhs.m20,
                m10 * rhs.m01 + m11 * rhs.m11 + m12 * rhs.m21,
                m10 * rhs.m02 + m11 * rhs.m12 + m12 * rhs.m22,
                m20 * rhs.m00 + m21 * rhs.m10 + m22 * rhs.m20,
                m20 * rhs.m01 + m21 * rhs.m11 + m22 * rhs.m21,
                m20 * rhs.m02 + m21 * rhs.m12 + m22 * rhs.m22};
        }
        Matrix3 operator/(Float const scalar) const
        {
            WORSE_ASSERT(scalar != 0.0f);
            Float const inv = 1.0f / scalar;
            return Matrix3{m00 * inv, m01 * inv, m02 * inv, m10 * inv, m11 * inv, m12 * inv, m20 * inv, m21 * inv, m22 * inv};
        }

        Matrix3& operator+=(Float const scalar)
        {
            m00 += scalar;
            m01 += scalar;
            m02 += scalar;
            m10 += scalar;
            m11 += scalar;
            m12 += scalar;
            m20 += scalar;
            m21 += scalar;
            m22 += scalar;
            return *this;
        }
        Matrix3& operator+=(Matrix3 const& rhs)
        {
            m00 += rhs.m00;
            m01 += rhs.m01;
            m02 += rhs.m02;
            m10 += rhs.m10;
            m11 += rhs.m11;
            m12 += rhs.m12;
            m20 += rhs.m20;
            m21 += rhs.m21;
            m22 += rhs.m22;
            return *this;
        }
        Matrix3& operator-=(Float const scalar)
        {
            m00 -= scalar;
            m01 -= scalar;
            m02 -= scalar;
            m10 -= scalar;
            m11 -= scalar;
            m12 -= scalar;
            m20 -= scalar;
            m21 -= scalar;
            m22 -= scalar;
            return *this;
        }
        Matrix3& operator-=(Matrix3 const& rhs)
        {
            m00 -= rhs.m00;
            m01 -= rhs.m01;
            m02 -= rhs.m02;
            m10 -= rhs.m10;
            m11 -= rhs.m11;
            m12 -= rhs.m12;
            m20 -= rhs.m20;
            m21 -= rhs.m21;
            m22 -= rhs.m22;
            return *this;
        }
        Matrix3& operator*=(Float const scalar)
        {
            m00 *= scalar;
            m01 *= scalar;
            m02 *= scalar;
            m10 *= scalar;
            m11 *= scalar;
            m12 *= scalar;
            m20 *= scalar;
            m21 *= scalar;
            m22 *= scalar;
            return *this;
        }
        Matrix3& operator*=(Matrix3 const& rhs)
        {
            Float const t00 = m00 * rhs.m00 + m01 * rhs.m10 + m02 * rhs.m20;
            Float const t01 = m00 * rhs.m01 + m01 * rhs.m11 + m02 * rhs.m21;
            Float const t02 = m00 * rhs.m02 + m01 * rhs.m12 + m02 * rhs.m22;
            Float const t10 = m10 * rhs.m00 + m11 * rhs.m10 + m12 * rhs.m20;
            Float const t11 = m10 * rhs.m01 + m11 * rhs.m11 + m12 * rhs.m21;
            Float const t12 = m10 * rhs.m02 + m11 * rhs.m12 + m12 * rhs.m22;
            Float const t20 = m20 * rhs.m00 + m21 * rhs.m10 + m22 * rhs.m20;
            Float const t21 = m20 * rhs.m01 + m21 * rhs.m11 + m22 * rhs.m21;
            Float const t22 = m20 * rhs.m02 + m21 * rhs.m12 + m22 * rhs.m22;
            m00             = t00;
            m01             = t01;
            m02             = t02;
            m10             = t10;
            m11             = t11;
            m12             = t12;
            m20             = t20;
            m21             = t21;
            m22             = t22;
            return *this;
        }
        Matrix3& operator/=(Float const scalar)
        {
            WORSE_ASSERT(scalar != 0.0f);
            Float const inv = 1.0f / scalar;
            m00 *= inv;
            m01 *= inv;
            m02 *= inv;
            m10 *= inv;
            m11 *= inv;
            m12 *= inv;
            m20 *= inv;
            m21 *= inv;
            m22 *= inv;
            return *this;
        }

        Vector3 operator*(Vector3 const& v) const
        {
            return Vector3{m00 * v.x + m01 * v.y + m02 * v.z,
                           m10 * v.x + m11 * v.y + m12 * v.z,
                           m20 * v.x + m21 * v.y + m22 * v.z};
        }

        WORSE_FORCE_INLINE friend Float Det(Matrix3 const& m)
        {
            return m.m00 * (m.m11 * m.m22 - m.m21 * m.m12) - m.m10 * (m.m01 * m.m22 - m.m21 * m.m02) + m.m20 * (m.m01 * m.m12 - m.m11 * m.m02);
        }

        WORSE_FORCE_INLINE friend Matrix3 Inverse(Matrix3 const& m)
        {
            Float const d = Det(m);
            if (Eq(d, 0.0f))
            {
                return NANM;
            }
            Float const inv = 1.0f / d;
            return Matrix3{
                (m.m11 * m.m22 - m.m12 * m.m21) * inv,
                (m.m02 * m.m21 - m.m01 * m.m22) * inv,
                (m.m01 * m.m12 - m.m02 * m.m11) * inv,
                (m.m12 * m.m20 - m.m10 * m.m22) * inv,
                (m.m00 * m.m22 - m.m02 * m.m20) * inv,
                (m.m02 * m.m10 - m.m00 * m.m12) * inv,
                (m.m10 * m.m21 - m.m11 * m.m20) * inv,
                (m.m01 * m.m20 - m.m00 * m.m21) * inv,
                (m.m00 * m.m11 - m.m01 * m.m10) * inv};
        }

        WORSE_FORCE_INLINE friend Matrix3 Transpose(Matrix3 const& m)
        {
            return Matrix3{m.m00, m.m10, m.m20, m.m01, m.m11, m.m21, m.m02, m.m12, m.m22};
        }

        static Matrix3 const IDENTITY;
        static Matrix3 const ZERO;
        static Matrix3 const NANM;
    };

    class Matrix4
    {
    public:
        union
        {
            struct
            {
                Float m00, m10, m20, m30;
                Float m01, m11, m21, m31;
                Float m02, m12, m22, m32;
                Float m03, m13, m23, m33;
            };
            Float data[16];
        };

    public:
        constexpr Matrix4()
            : m00{0.0f}, m10{0.0f}, m20{0.0f}, m30{0.0f}, m01{0.0f}, m11{0.0f}, m21{0.0f}, m31{0.0f}, m02{0.0f}, m12{0.0f}, m22{0.0f}, m32{0.0f}, m03{0.0f}, m13{0.0f}, m23{0.0f}, m33{1.0f}
        {
        }

        constexpr Matrix4(Float const m00, Float const m01, Float const m02, Float const m03,
                          Float const m10, Float const m11, Float const m12, Float const m13,
                          Float const m20, Float const m21, Float const m22, Float const m23,
                          Float const m30, Float const m31, Float const m32, Float const m33)
            : m00{m00}, m01{m01}, m02{m02}, m03{m03}, m10{m10}, m11{m11}, m12{m12}, m13{m13}, m20{m20}, m21{m21}, m22{m22}, m23{m23}, m30{m30}, m31{m31}, m32{m32}, m33{m33}
        {
        }

        constexpr Matrix4(Vector4 const& col0, Vector4 const& col1, Vector4 const& col2, Vector4 const& col3)
            : m00{col0.x}, m10{col0.y}, m20{col0.z}, m30{col0.w}, m01{col1.x}, m11{col1.y}, m21{col1.z}, m31{col1.w}, m02{col2.x}, m12{col2.y}, m22{col2.z}, m32{col2.w}, m03{col3.x}, m13{col3.y}, m23{col3.z}, m33{col3.w}
        {
        }

        Matrix4(Matrix3 const& mat3)
            : m00{mat3.m00}, m10{mat3.m10}, m20{mat3.m20}, m30{0.0f}, m01{mat3.m01}, m11{mat3.m11}, m21{mat3.m21}, m31{0.0f}, m02{mat3.m02}, m12{mat3.m12}, m22{mat3.m22}, m32{0.0f}, m03{0.0f}, m13{0.0f}, m23{0.0f}, m33{1.0f}
        {
        }

        WORSE_FORCE_INLINE explicit operator Matrix3() const
        {
            return Matrix3{m00, m01, m02, m10, m11, m12, m20, m21, m22};
        }

        Float* ptr()
        {
            return &m00;
        }
        Float const* ptr() const
        {
            return &m00;
        }

        Vector4& operator[](Size const index)
        {
            WORSE_ASSERT((index >= 0) && (index < 4));
            return reinterpret_cast<Vector4*>(ptr())[index];
        }
        Vector4 const& operator[](Size const index) const
        {
            WORSE_ASSERT((index >= 0) && (index < 4));
            return reinterpret_cast<const Vector4*>(ptr())[index];
        }

        Bool operator==(Matrix4 const& other) const
        {
            return Eq(m00, other.m00) && Eq(m10, other.m10) && Eq(m20, other.m20) && Eq(m30, other.m30) &&
                   Eq(m01, other.m01) && Eq(m11, other.m11) && Eq(m21, other.m21) && Eq(m31, other.m31) &&
                   Eq(m02, other.m02) && Eq(m12, other.m12) && Eq(m22, other.m22) && Eq(m32, other.m32) &&
                   Eq(m03, other.m03) && Eq(m13, other.m13) && Eq(m23, other.m23) && Eq(m33, other.m33);
        }
        Bool operator!=(Matrix4 const& other) const
        {
            return Neq(m00, other.m00) || Neq(m10, other.m10) || Neq(m20, other.m20) || Neq(m30, other.m30) ||
                   Neq(m01, other.m01) || Neq(m11, other.m11) || Neq(m21, other.m21) || Neq(m31, other.m31) ||
                   Neq(m02, other.m02) || Neq(m12, other.m12) || Neq(m22, other.m22) || Neq(m32, other.m32) ||
                   Neq(m03, other.m03) || Neq(m13, other.m13) || Neq(m23, other.m23) || Neq(m33, other.m33);
        }

        Matrix4 operator+() const
        {
            return *this;
        }
        Matrix4 operator-() const
        {
            return Matrix4{-m00, -m01, -m02, -m03, -m10, -m11, -m12, -m13, -m20, -m21, -m22, -m23, -m30, -m31, -m32, -m33};
        }

        Matrix4 operator+(Float const scalar) const
        {
            return Matrix4{m00 + scalar, m01 + scalar, m02 + scalar, m03 + scalar, m10 + scalar, m11 + scalar, m12 + scalar, m13 + scalar, m20 + scalar, m21 + scalar, m22 + scalar, m23 + scalar, m30 + scalar, m31 + scalar, m32 + scalar, m33 + scalar};
        }
        Matrix4 operator+(Matrix4 const& rhs) const
        {
            return Matrix4{m00 + rhs.m00, m01 + rhs.m01, m02 + rhs.m02, m03 + rhs.m03, m10 + rhs.m10, m11 + rhs.m11, m12 + rhs.m12, m13 + rhs.m13, m20 + rhs.m20, m21 + rhs.m21, m22 + rhs.m22, m23 + rhs.m23, m30 + rhs.m30, m31 + rhs.m31, m32 + rhs.m32, m33 + rhs.m33};
        }
        Matrix4 operator-(Float const scalar) const
        {
            return Matrix4{m00 - scalar, m01 - scalar, m02 - scalar, m03 - scalar, m10 - scalar, m11 - scalar, m12 - scalar, m13 - scalar, m20 - scalar, m21 - scalar, m22 - scalar, m23 - scalar, m30 - scalar, m31 - scalar, m32 - scalar, m33 - scalar};
        }
        Matrix4 operator-(Matrix4 const& rhs) const
        {
            return Matrix4{m00 - rhs.m00, m01 - rhs.m01, m02 - rhs.m02, m03 - rhs.m03, m10 - rhs.m10, m11 - rhs.m11, m12 - rhs.m12, m13 - rhs.m13, m20 - rhs.m20, m21 - rhs.m21, m22 - rhs.m22, m23 - rhs.m23, m30 - rhs.m30, m31 - rhs.m31, m32 - rhs.m32, m33 - rhs.m33};
        }
        Matrix4 operator*(Float const scalar) const
        {
            return Matrix4{m00 * scalar, m01 * scalar, m02 * scalar, m03 * scalar, m10 * scalar, m11 * scalar, m12 * scalar, m13 * scalar, m20 * scalar, m21 * scalar, m22 * scalar, m23 * scalar, m30 * scalar, m31 * scalar, m32 * scalar, m33 * scalar};
        }
        Matrix4 operator*(Matrix4 const& rhs) const
        {
            return Matrix4{
                m00 * rhs.m00 + m01 * rhs.m10 + m02 * rhs.m20 + m03 * rhs.m30,
                m00 * rhs.m01 + m01 * rhs.m11 + m02 * rhs.m21 + m03 * rhs.m31,
                m00 * rhs.m02 + m01 * rhs.m12 + m02 * rhs.m22 + m03 * rhs.m32,
                m00 * rhs.m03 + m01 * rhs.m13 + m02 * rhs.m23 + m03 * rhs.m33,
                m10 * rhs.m00 + m11 * rhs.m10 + m12 * rhs.m20 + m13 * rhs.m30,
                m10 * rhs.m01 + m11 * rhs.m11 + m12 * rhs.m21 + m13 * rhs.m31,
                m10 * rhs.m02 + m11 * rhs.m12 + m12 * rhs.m22 + m13 * rhs.m32,
                m10 * rhs.m03 + m11 * rhs.m13 + m12 * rhs.m23 + m13 * rhs.m33,
                m20 * rhs.m00 + m21 * rhs.m10 + m22 * rhs.m20 + m23 * rhs.m30,
                m20 * rhs.m01 + m21 * rhs.m11 + m22 * rhs.m21 + m23 * rhs.m31,
                m20 * rhs.m02 + m21 * rhs.m12 + m22 * rhs.m22 + m23 * rhs.m32,
                m20 * rhs.m03 + m21 * rhs.m13 + m22 * rhs.m23 + m23 * rhs.m33,
                m30 * rhs.m00 + m31 * rhs.m10 + m32 * rhs.m20 + m33 * rhs.m30,
                m30 * rhs.m01 + m31 * rhs.m11 + m32 * rhs.m21 + m33 * rhs.m31,
                m30 * rhs.m02 + m31 * rhs.m12 + m32 * rhs.m22 + m33 * rhs.m32,
                m30 * rhs.m03 + m31 * rhs.m13 + m32 * rhs.m23 + m33 * rhs.m33};
        }
        Matrix4 operator/(Float const scalar) const
        {
            WORSE_ASSERT(scalar != 0.0f);
            Float const inv = 1.0f / scalar;
            return Matrix4{m00 * inv, m01 * inv, m02 * inv, m03 * inv, m10 * inv, m11 * inv, m12 * inv, m13 * inv, m20 * inv, m21 * inv, m22 * inv, m23 * inv, m30 * inv, m31 * inv, m32 * inv, m33 * inv};
        }

        Matrix4& operator+=(Float const scalar)
        {
            m00 += scalar;
            m01 += scalar;
            m02 += scalar;
            m03 += scalar;
            m10 += scalar;
            m11 += scalar;
            m12 += scalar;
            m13 += scalar;
            m20 += scalar;
            m21 += scalar;
            m22 += scalar;
            m23 += scalar;
            m30 += scalar;
            m31 += scalar;
            m32 += scalar;
            m33 += scalar;
            return *this;
        }
        Matrix4& operator+=(Matrix4 const& rhs)
        {
            m00 += rhs.m00;
            m01 += rhs.m01;
            m02 += rhs.m02;
            m03 += rhs.m03;
            m10 += rhs.m10;
            m11 += rhs.m11;
            m12 += rhs.m12;
            m13 += rhs.m13;
            m20 += rhs.m20;
            m21 += rhs.m21;
            m22 += rhs.m22;
            m23 += rhs.m23;
            m30 += rhs.m30;
            m31 += rhs.m31;
            m32 += rhs.m32;
            m33 += rhs.m33;
            return *this;
        }
        Matrix4& operator-=(Float const scalar)
        {
            m00 -= scalar;
            m01 -= scalar;
            m02 -= scalar;
            m03 -= scalar;
            m10 -= scalar;
            m11 -= scalar;
            m12 -= scalar;
            m13 -= scalar;
            m20 -= scalar;
            m21 -= scalar;
            m22 -= scalar;
            m23 -= scalar;
            m30 -= scalar;
            m31 -= scalar;
            m32 -= scalar;
            m33 -= scalar;
            return *this;
        }
        Matrix4& operator-=(Matrix4 const& rhs)
        {
            m00 -= rhs.m00;
            m01 -= rhs.m01;
            m02 -= rhs.m02;
            m03 -= rhs.m03;
            m10 -= rhs.m10;
            m11 -= rhs.m11;
            m12 -= rhs.m12;
            m13 -= rhs.m13;
            m20 -= rhs.m20;
            m21 -= rhs.m21;
            m22 -= rhs.m22;
            m23 -= rhs.m23;
            m30 -= rhs.m30;
            m31 -= rhs.m31;
            m32 -= rhs.m32;
            m33 -= rhs.m33;
            return *this;
        }
        Matrix4& operator*=(Float const scalar)
        {
            m00 *= scalar;
            m01 *= scalar;
            m02 *= scalar;
            m03 *= scalar;
            m10 *= scalar;
            m11 *= scalar;
            m12 *= scalar;
            m13 *= scalar;
            m20 *= scalar;
            m21 *= scalar;
            m22 *= scalar;
            m23 *= scalar;
            m30 *= scalar;
            m31 *= scalar;
            m32 *= scalar;
            m33 *= scalar;
            return *this;
        }
        Matrix4& operator*=(Matrix4 const& rhs)
        {
            Float const t00 = m00 * rhs.m00 + m01 * rhs.m10 + m02 * rhs.m20 + m03 * rhs.m30;
            Float const t01 = m00 * rhs.m01 + m01 * rhs.m11 + m02 * rhs.m21 + m03 * rhs.m31;
            Float const t02 = m00 * rhs.m02 + m01 * rhs.m12 + m02 * rhs.m22 + m03 * rhs.m32;
            Float const t03 = m00 * rhs.m03 + m01 * rhs.m13 + m02 * rhs.m23 + m03 * rhs.m33;
            Float const t10 = m10 * rhs.m00 + m11 * rhs.m10 + m12 * rhs.m20 + m13 * rhs.m30;
            Float const t11 = m10 * rhs.m01 + m11 * rhs.m11 + m12 * rhs.m21 + m13 * rhs.m31;
            Float const t12 = m10 * rhs.m02 + m11 * rhs.m12 + m12 * rhs.m22 + m13 * rhs.m32;
            Float const t13 = m10 * rhs.m03 + m11 * rhs.m13 + m12 * rhs.m23 + m13 * rhs.m33;
            Float const t20 = m20 * rhs.m00 + m21 * rhs.m10 + m22 * rhs.m20 + m23 * rhs.m30;
            Float const t21 = m20 * rhs.m01 + m21 * rhs.m11 + m22 * rhs.m21 + m23 * rhs.m31;
            Float const t22 = m20 * rhs.m02 + m21 * rhs.m12 + m22 * rhs.m22 + m23 * rhs.m32;
            Float const t23 = m20 * rhs.m03 + m21 * rhs.m13 + m22 * rhs.m23 + m23 * rhs.m33;
            Float const t30 = m30 * rhs.m00 + m31 * rhs.m10 + m32 * rhs.m20 + m33 * rhs.m30;
            Float const t31 = m30 * rhs.m01 + m31 * rhs.m11 + m32 * rhs.m21 + m33 * rhs.m31;
            Float const t32 = m30 * rhs.m02 + m31 * rhs.m12 + m32 * rhs.m22 + m33 * rhs.m32;
            Float const t33 = m30 * rhs.m03 + m31 * rhs.m13 + m32 * rhs.m23 + m33 * rhs.m33;
            m00             = t00;
            m01             = t01;
            m02             = t02;
            m03             = t03;
            m10             = t10;
            m11             = t11;
            m12             = t12;
            m13             = t13;
            m20             = t20;
            m21             = t21;
            m22             = t22;
            m23             = t23;
            m30             = t30;
            m31             = t31;
            m32             = t32;
            m33             = t33;
            return *this;
        }
        Matrix4& operator/=(Float const scalar)
        {
            WORSE_ASSERT(scalar != 0.0f);
            Float const inv = 1.0f / scalar;
            m00 *= inv;
            m01 *= inv;
            m02 *= inv;
            m03 *= inv;
            m10 *= inv;
            m11 *= inv;
            m12 *= inv;
            m13 *= inv;
            m20 *= inv;
            m21 *= inv;
            m22 *= inv;
            m23 *= inv;
            m30 *= inv;
            m31 *= inv;
            m32 *= inv;
            m33 *= inv;
            return *this;
        }

        Vector4 operator*(Vector4 const& v) const
        {
            return Vector4{m00 * v.x + m01 * v.y + m02 * v.z + m03 * v.w,
                           m10 * v.x + m11 * v.y + m12 * v.z + m13 * v.w,
                           m20 * v.x + m21 * v.y + m22 * v.z + m23 * v.w,
                           m30 * v.x + m31 * v.y + m32 * v.z + m33 * v.w};
        }

        WORSE_FORCE_INLINE friend Float Det(Matrix4 const& m)
        {
            return m.m00 * (m.m11 * (m.m22 * m.m33 - m.m32 * m.m23) - m.m21 * (m.m12 * m.m33 - m.m32 * m.m13) + m.m31 * (m.m12 * m.m23 - m.m22 * m.m13)) -
                   m.m10 * (m.m01 * (m.m22 * m.m33 - m.m32 * m.m23) - m.m21 * (m.m02 * m.m33 - m.m32 * m.m03) + m.m31 * (m.m02 * m.m23 - m.m22 * m.m03)) +
                   m.m20 * (m.m01 * (m.m12 * m.m33 - m.m32 * m.m13) - m.m11 * (m.m02 * m.m33 - m.m32 * m.m03) + m.m31 * (m.m02 * m.m13 - m.m12 * m.m03)) -
                   m.m30 * (m.m01 * (m.m12 * m.m23 - m.m22 * m.m13) - m.m11 * (m.m02 * m.m23 - m.m22 * m.m03) + m.m21 * (m.m02 * m.m13 - m.m12 * m.m03));
        }

        WORSE_FORCE_INLINE friend Matrix4 Inverse(Matrix4 const& m)
        {
            Float const d = Det(m);
            if (Eq(d, 0.0f))
            {
                return NANM;
            }
            Float const invD = 1.0f / d;
            return Matrix4{
                (m.m11 * m.m22 * m.m33 + m.m12 * m.m23 * m.m31 + m.m13 * m.m21 * m.m32 - m.m13 * m.m22 * m.m31 - m.m11 * m.m23 * m.m32 - m.m12 * m.m21 * m.m33) * invD,
                -(m.m01 * m.m22 * m.m33 + m.m02 * m.m23 * m.m31 + m.m03 * m.m21 * m.m32 - m.m03 * m.m22 * m.m31 - m.m01 * m.m23 * m.m32 - m.m02 * m.m21 * m.m33) * invD,
                (m.m01 * m.m12 * m.m33 + m.m02 * m.m13 * m.m31 + m.m03 * m.m11 * m.m32 - m.m03 * m.m12 * m.m31 - m.m01 * m.m13 * m.m32 - m.m02 * m.m11 * m.m33) * invD,
                -(m.m01 * m.m12 * m.m23 + m.m02 * m.m13 * m.m21 + m.m03 * m.m11 * m.m22 - m.m03 * m.m12 * m.m21 - m.m01 * m.m13 * m.m22 - m.m02 * m.m11 * m.m23) * invD,

                -(m.m10 * m.m22 * m.m33 + m.m12 * m.m23 * m.m30 + m.m13 * m.m20 * m.m32 - m.m13 * m.m22 * m.m30 - m.m10 * m.m23 * m.m32 - m.m12 * m.m20 * m.m33) * invD,
                (m.m00 * m.m22 * m.m33 + m.m02 * m.m23 * m.m30 + m.m03 * m.m20 * m.m32 - m.m03 * m.m22 * m.m30 - m.m00 * m.m23 * m.m32 - m.m02 * m.m20 * m.m33) * invD,
                -(m.m00 * m.m12 * m.m33 + m.m02 * m.m13 * m.m30 + m.m03 * m.m10 * m.m32 - m.m03 * m.m12 * m.m30 - m.m00 * m.m13 * m.m32 - m.m02 * m.m10 * m.m33) * invD,
                (m.m00 * m.m12 * m.m23 + m.m02 * m.m13 * m.m20 + m.m03 * m.m10 * m.m22 - m.m03 * m.m12 * m.m20 - m.m00 * m.m13 * m.m22 - m.m02 * m.m10 * m.m23) * invD,

                (m.m10 * m.m21 * m.m33 + m.m11 * m.m23 * m.m30 + m.m13 * m.m20 * m.m31 - m.m13 * m.m21 * m.m30 - m.m10 * m.m23 * m.m31 - m.m11 * m.m20 * m.m33) * invD,
                -(m.m00 * m.m21 * m.m33 + m.m01 * m.m23 * m.m30 + m.m03 * m.m20 * m.m31 - m.m03 * m.m21 * m.m30 - m.m00 * m.m23 * m.m31 - m.m01 * m.m20 * m.m33) * invD,
                (m.m00 * m.m11 * m.m33 + m.m01 * m.m13 * m.m30 + m.m03 * m.m10 * m.m31 - m.m03 * m.m11 * m.m30 - m.m00 * m.m13 * m.m31 - m.m01 * m.m10 * m.m33) * invD,
                -(m.m00 * m.m11 * m.m23 + m.m01 * m.m13 * m.m20 + m.m03 * m.m10 * m.m21 - m.m03 * m.m11 * m.m20 - m.m00 * m.m13 * m.m21 - m.m01 * m.m10 * m.m23) * invD,

                -(m.m10 * m.m21 * m.m32 + m.m11 * m.m22 * m.m30 + m.m12 * m.m20 * m.m31 - m.m12 * m.m21 * m.m30 - m.m10 * m.m22 * m.m31 - m.m11 * m.m20 * m.m32) * invD,
                (m.m00 * m.m21 * m.m32 + m.m01 * m.m22 * m.m30 + m.m02 * m.m20 * m.m31 - m.m02 * m.m21 * m.m30 - m.m00 * m.m22 * m.m31 - m.m01 * m.m20 * m.m32) * invD,
                -(m.m00 * m.m11 * m.m32 + m.m01 * m.m12 * m.m30 + m.m02 * m.m10 * m.m31 - m.m02 * m.m11 * m.m30 - m.m00 * m.m12 * m.m31 - m.m01 * m.m10 * m.m32) * invD,
                (m.m00 * m.m11 * m.m22 + m.m01 * m.m12 * m.m20 + m.m02 * m.m10 * m.m21 - m.m02 * m.m11 * m.m20 - m.m00 * m.m12 * m.m21 - m.m01 * m.m10 * m.m22) * invD};
        }

        WORSE_FORCE_INLINE friend Matrix4 Transpose(Matrix4 const& m)
        {
            return Matrix4{m.m00, m.m10, m.m20, m.m30, m.m01, m.m11, m.m21, m.m31, m.m02, m.m12, m.m22, m.m32, m.m03, m.m13, m.m23, m.m33};
        }

        static Matrix4 const IDENTITY;
        static Matrix4 const ZERO;
        static Matrix4 const NANM;
    };

} // namespace Worse