#pragma once
#include "Base.hpp"
#include "Vector.hpp"

namespace worse
{
    // clang-format off

    /// 2x2 Column major matrix
    struct Matrix2
    {
        union
        {
            struct
            {
                float m00, m10;
                float m01, m11;
            };
            struct { Vector2 col0, col1; };
            float data[4];
        };

        constexpr Matrix2()
            : m00(0.0f), m10(0.0f)
            , m01(0.0f), m11(0.0f) {}

        constexpr Matrix2(float m00, float m01,
                          float m10, float m11)
            : m00(m00), m01(m01)
            , m10(m10), m11(m11) {}

        constexpr Matrix2(Vector2 const& col0, Vector2 const& col1)
            : col0(col0), col1(col1) {}

        Matrix2(float* col[2])
            : m00(col[0][0]), m01(col[1][0])
            , m10(col[0][1]), m11(col[1][1]) {}

        Matrix2(float* raw) { std::memcpy(data, raw, sizeof(float) * 4); }

        Matrix2 operator+(Matrix2 const& m) const { return Matrix2(col0 + m.col0, col1 + m.col1); }
        Matrix2 operator-(Matrix2 const& m) const { return Matrix2(col0 - m.col0, col1 - m.col1); }
        Matrix2 operator*(float const s) const    { return Matrix2(col0 * s, col1 * s); }
        Vector2 operator*(Vector2 const& v) const
        {
            return Vector2(
                m00 * v.x + m01 * v.y,
                m10 * v.x + m11 * v.y
            );
        }
        Matrix2 operator*(Matrix2 const& m) const
        {
            return Matrix2(
                m00 * m.m00 + m01 * m.m10,
                m00 * m.m01 + m01 * m.m11,
                m10 * m.m00 + m11 * m.m10,
                m10 * m.m01 + m11 * m.m11
            );
        }
        float& operator[](std::size_t const index) { return data[index]; }

        static constexpr Matrix2 IDENTITY() { return Matrix2(Vector2::X(), Vector2::Y()); }
        static constexpr Matrix2 ZERO()     { return Matrix2(Vector2::ZERO(), Vector2::ZERO()); }
        static constexpr Matrix2 NANM()     { return Matrix2(Vector2::NANM(), Vector2::NANM()); }
    };

    /// 3x3 Column major matrix
    struct Matrix3
    {
        union
        {
            struct
            {
                float m00, m10, m20;
                float m01, m11, m21;
                float m02, m12, m22;
            };
            struct { Vector3 col0, col1, col2; };
            float data[9];
        };

        constexpr Matrix3()
            : m00(0.0f), m10(0.0f), m20(0.0f)
            , m01(0.0f), m11(0.0f), m21(0.0f)
            , m02(0.0f), m12(0.0f), m22(0.0f) {}

        constexpr Matrix3(float m00, float m01, float m02,
                          float m10, float m11, float m12,
                          float m20, float m21, float m22)
            : m00(m00), m01(m01), m02(m02)
            , m10(m10), m11(m11), m12(m12)
            , m20(m20), m21(m21), m22(m22) {}

        constexpr Matrix3(Vector3 const& col0, Vector3 const& col1, Vector3 const& col2)
            : col0(col0), col1(col1), col2(col2) {}

        Matrix3(float* col[3])
            : m00(col[0][0]), m01(col[1][0]), m02(col[2][0])
            , m10(col[0][1]), m11(col[1][1]), m12(col[2][1])
            , m20(col[0][2]), m21(col[1][2]), m22(col[2][2]) {}

        Matrix3(float* raw)
        {
            std::memcpy(data, raw, sizeof(float) * 9);
        }

        Matrix3 operator+(Matrix3 const& m) const { return Matrix3(col0 + m.col0, col1 + m.col1, col2 + m.col2); }
        Matrix3 operator-(Matrix3 const& m) const { return Matrix3(col0 - m.col0, col1 - m.col1, col2 - m.col2); }
        Matrix3 operator*(float const s) const    { return Matrix3(col0 * s, col1 * s, col2 * s); }
        Vector3 operator*(Vector3 const& v) const
        {
            return Vector3(
                m00 * v.x + m01 * v.y + m02 * v.z,
                m10 * v.x + m11 * v.y + m12 * v.z,
                m20 * v.x + m21 * v.y + m22 * v.z
            );
        }
        Matrix3 operator*(Matrix3 const& m) const
        {
            return Matrix3(
                m00 * m.m00 + m01 * m.m10 + m02 * m.m20, m00 * m.m01 + m01 * m.m11 + m02 * m.m21, m00 * m.m02 + m01 * m.m12 + m02 * m.m22,
                m10 * m.m00 + m11 * m.m10 + m12 * m.m20, m10 * m.m01 + m11 * m.m11 + m12 * m.m21, m10 * m.m02 + m11 * m.m12 + m12 * m.m22,
                m20 * m.m00 + m21 * m.m10 + m22 * m.m20, m20 * m.m01 + m21 * m.m11 + m22 * m.m21, m20 * m.m02 + m21 * m.m12 + m22 * m.m22
            );
        }
        float& operator[](std::size_t const index) { return data[index]; }

        static constexpr Matrix3 IDENTITY() { return Matrix3(Vector3::X(), Vector3::Y(), Vector3::Z()); }
        static constexpr Matrix3 ZERO()     { return Matrix3(Vector3::ZERO(), Vector3::ZERO(), Vector3::ZERO()); }
        static constexpr Matrix3 NANM()     { return Matrix3(Vector3::NANM(), Vector3::NANM(), Vector3::NANM()); }
    };

    /// 4x4 Column major matrix
    struct Matrix4
    {
        union
        {
            struct
            {
                float m00, m10, m20, m30;
                float m01, m11, m21, m31;
                float m02, m12, m22, m32;
                float m03, m13, m23, m33;
            };
            struct { Vector4 col0, col1, col2, col3; };
            float data[16];
        };

        constexpr Matrix4()
            : m00(0.0f), m10(0.0f), m20(0.0f), m30(0.0f)
            , m01(0.0f), m11(0.0f), m21(0.0f), m31(0.0f)
            , m02(0.0f), m12(0.0f), m22(0.0f), m32(0.0f)
            , m03(0.0f), m13(0.0f), m23(0.0f), m33(1.0f) {}

        constexpr Matrix4(float m00, float m01, float m02, float m03,
                          float m10, float m11, float m12, float m13,
                          float m20, float m21, float m22, float m23,
                          float m30, float m31, float m32, float m33)
            : m00(m00), m01(m01), m02(m02), m03(m03)
            , m10(m10), m11(m11), m12(m12), m13(m13)
            , m20(m20), m21(m21), m22(m22), m23(m23)
            , m30(m30), m31(m31), m32(m32), m33(m33) {}

        constexpr Matrix4(Vector4 const& col0, Vector4 const& col1, Vector4 const& col2, Vector4 const& col3)
            : col0(col0), col1(col1), col2(col2), col3(col3) {}

        Matrix4(float* col[4])
            : m00(col[0][0]), m01(col[1][0]), m02(col[2][0]), m03(col[3][0]),
              m10(col[0][1]), m11(col[1][1]), m12(col[2][1]), m13(col[3][1]),
              m20(col[0][2]), m21(col[1][2]), m22(col[2][2]), m23(col[3][2]),
              m30(col[0][3]), m31(col[1][3]), m32(col[2][3]), m33(col[3][3]) {}

        Matrix4(float* raw) { std::memcpy(data, raw, sizeof(float) * 16); }

        Matrix4(Matrix3 mat3)
            : m00(mat3.m00), m01(mat3.m01), m02(mat3.m02), m03(0)
            , m10(mat3.m10), m11(mat3.m11), m12(mat3.m12), m13(0)
            , m20(mat3.m20), m21(mat3.m21), m22(mat3.m22), m23(0)
            , m30(0),        m31(0),        m32(0),        m33(1) {}

        Matrix3 toMat3() const { return Matrix3(m00, m01, m02, m10, m11, m12, m20, m21, m22); }

        Matrix4 operator+(Matrix4 const& m) const { return Matrix4(col0 + m.col0, col1 + m.col1, col2 + m.col2, col3 + m.col3); }
        Matrix4 operator-(Matrix4 const& m) const { return Matrix4(col0 - m.col0, col1 - m.col1, col2 - m.col2, col3 - m.col3); }
        Matrix4 operator*(float const s) const    { return Matrix4(col0 * s, col1 * s, col2 * s, col3 * s); }
        Vector4 operator*(Vector4 const& v) const
        {
            return Vector4(
                m00 * v.x + m01 * v.y + m02 * v.z + m03 * v.w,
                m10 * v.x + m11 * v.y + m12 * v.z + m13 * v.w,
                m20 * v.x + m21 * v.y + m22 * v.z + m23 * v.w,
                m30 * v.x + m31 * v.y + m32 * v.z + m33 * v.w
            );
        }
        /// Mat4 matrix multiply
        Matrix4 operator*(Matrix4 const& m) const
        {
            return Matrix4(
                m00 * m.m00 + m01 * m.m10 + m02 * m.m20 + m03 * m.m30,
                m00 * m.m01 + m01 * m.m11 + m02 * m.m21 + m03 * m.m31,
                m00 * m.m02 + m01 * m.m12 + m02 * m.m22 + m03 * m.m32,
                m00 * m.m03 + m01 * m.m13 + m02 * m.m23 + m03 * m.m33,
   
                m10 * m.m00 + m11 * m.m10 + m12 * m.m20 + m13 * m.m30,
                m10 * m.m01 + m11 * m.m11 + m12 * m.m21 + m13 * m.m31,
                m10 * m.m02 + m11 * m.m12 + m12 * m.m22 + m13 * m.m32,
                m10 * m.m03 + m11 * m.m13 + m12 * m.m23 + m13 * m.m33,
   
                m20 * m.m00 + m21 * m.m10 + m22 * m.m20 + m23 * m.m30,
                m20 * m.m01 + m21 * m.m11 + m22 * m.m21 + m23 * m.m31,
                m20 * m.m02 + m21 * m.m12 + m22 * m.m22 + m23 * m.m32,
                m20 * m.m03 + m21 * m.m13 + m22 * m.m23 + m23 * m.m33,
   
                m30 * m.m00 + m31 * m.m10 + m32 * m.m20 + m33 * m.m30,
                m30 * m.m01 + m31 * m.m11 + m32 * m.m21 + m33 * m.m31,
                m30 * m.m02 + m31 * m.m12 + m32 * m.m22 + m33 * m.m32,
                m30 * m.m03 + m31 * m.m13 + m32 * m.m23 + m33 * m.m33
            );
        }
        float& operator[](std::size_t const index) { return data[index]; }

        static constexpr Matrix4 IDENTITY() { return Matrix4(Vector4::X(), Vector4::Y(), Vector4::Z(), Vector4::W()); }
        static constexpr Matrix4 ZERO()     { return Matrix4(Vector4::ZERO(), Vector4::ZERO(), Vector4::ZERO(), Vector4::ZERO()); }
        static constexpr Matrix4 NANM()     { return Matrix4(Vector4::NANM(), Vector4::NANM(), Vector4::NANM(), Vector4::NANM()); }
    };

    inline constexpr Matrix2 transpose(Matrix2 const& m)
    {
        return Matrix2(
            m.m00, m.m10,           
            m.m01, m.m11
        );
    }
    inline constexpr Matrix3 transpose(Matrix3 const& m)
    {
        return Matrix3(
            m.m00, m.m10, m.m20,           
            m.m01, m.m11, m.m21,
            m.m02, m.m12, m.m22
        );
    }
    inline constexpr Matrix4 transpose(Matrix4 const& m)
    {
        return Matrix4(
            m.m00, m.m10, m.m20, m.m30,           
            m.m01, m.m11, m.m21, m.m31,
            m.m02, m.m12, m.m22, m.m32,
            m.m03, m.m13, m.m23, m.m33
        );
    }

    inline constexpr float determinant(Matrix2 const& m)
    {
        return
            m.m00 * m.m11 -
            m.m01 * m.m10;
    }
    inline constexpr float determinant(Matrix3 const& m)
    {
        return
            m.m00 * ( m.m11 *  m.m22 -  m.m12 *  m.m21) - 
            m.m01 * ( m.m10 *  m.m22 -  m.m12 *  m.m20) +
            m.m02 * ( m.m10 *  m.m21 -  m.m11 *  m.m20);

    }
    inline constexpr float determinant(Matrix4 const& m)
    {
        return
            m.m00 * (m.m11 * m.m22 * m.m33 + m.m12 * m.m23 * m.m31 + m.m13 * m.m21 * m.m32 - m.m13 * m.m22 * m.m31 - m.m11 * m.m23 * m.m32 - m.m12 * m.m21 * m.m33) -    
            m.m01 * (m.m10 * m.m22 * m.m33 + m.m12 * m.m23 * m.m30 + m.m13 * m.m20 * m.m32 - m.m13 * m.m22 * m.m30 - m.m10 * m.m23 * m.m32 - m.m12 * m.m20 * m.m33) +
            m.m02 * (m.m10 * m.m21 * m.m33 + m.m11 * m.m23 * m.m30 + m.m13 * m.m20 * m.m31 - m.m13 * m.m21 * m.m30 - m.m10 * m.m23 * m.m31 - m.m11 * m.m20 * m.m33) -
            m.m03 * (m.m10 * m.m21 * m.m32 + m.m11 * m.m22 * m.m30 + m.m12 * m.m20 * m.m31 - m.m12 * m.m21 * m.m30 - m.m10 * m.m22 * m.m31 - m.m11 * m.m20 * m.m32);
    }

    inline constexpr float trace(Matrix2 const& m) { return m.m00 + m.m11; }
    inline constexpr float trace(Matrix3 const& m) { return m.m00 + m.m11 + m.m22; }
    inline constexpr float trace(Matrix4 const& m) { return m.m00 + m.m11 + m.m22 + m.m33; }

    inline constexpr Matrix2 inverse(Matrix2 const& m)
    {
        float det = determinant(m);
        if (det == 0)
        {
            return Matrix2::NANM();
        }
        float invDet = 1.0f / det;
        return Matrix2(
             m.m11 * invDet, -m.m01 * invDet,
            -m.m10 * invDet,  m.m00 * invDet);
    }
    inline constexpr Matrix3 inverse(Matrix3 const& m)
    {
        float det = determinant(m);
        if (det == 0)
        {
            return Matrix3::NANM();
        }
        float invDet = 1.0f / det;
        return Matrix3(
            (m.m11 * m.m22 - m.m12 * m.m21) * invDet, (m.m02 * m.m21 - m.m01 * m.m22) * invDet, (m.m01 * m.m12 - m.m02 * m.m11) * invDet,           
            (m.m12 * m.m20 - m.m10 * m.m22) * invDet, (m.m00 * m.m22 - m.m02 * m.m20) * invDet, (m.m02 * m.m10 - m.m00 * m.m12) * invDet,           
            (m.m10 * m.m21 - m.m11 * m.m20) * invDet, (m.m01 * m.m20 - m.m00 * m.m21) * invDet, (m.m00 * m.m11 - m.m01 * m.m10) * invDet
        );
    }
    inline constexpr Matrix4 inverse(Matrix4 const& m)
    {
        float det = determinant(m);
        if (det == 0)
        {
            return Matrix4::NANM();
        }
        float invDet = 1.0f / det;
        return Matrix4(
            (m.m11 * m.m22 * m.m33 + m.m12 * m.m23 * m.m31 + m.m13 * m.m21 * m.m32 - m.m13 * m.m22 * m.m31 - m.m11 * m.m23 * m.m32 - m.m12 * m.m21 * m.m33) * invDet,        
            (m.m03 * m.m22 * m.m31 + m.m01 * m.m23 * m.m32 + m.m02 * m.m21 * m.m33 - m.m02 * m.m22 * m.m31 - m.m03 * m.m23 * m.m32 - m.m01 * m.m21 * m.m33) * invDet,
            (m.m03 * m.m12 * m.m31 + m.m01 * m.m13 * m.m32 + m.m02 * m.m11 * m.m33 - m.m02 * m.m12 * m.m31 - m.m03 * m.m13 * m.m32 - m.m01 * m.m11 * m.m33) * invDet,
            (m.m03 * m.m12 * m.m21 + m.m01 * m.m13 * m.m22 + m.m02 * m.m11 * m.m23 - m.m02 * m.m12 * m.m21 - m.m03 * m.m13 * m.m22 - m.m01 * m.m11 * m.m23) * invDet,

            (m.m10 * m.m22 * m.m33 + m.m12 * m.m23 * m.m30 + m.m13 * m.m20 * m.m32 - m.m13 * m.m22 * m.m30 - m.m10 * m.m23 * m.m32 - m.m12 * m.m20 * m.m33) * invDet,
            (m.m02 * m.m22 * m.m30 + m.m00 * m.m23 * m.m32 + m.m03 * m.m20 * m.m32 - m.m03 * m.m22 * m.m30 - m.m02 * m.m23 * m.m32 - m.m00 * m.m20 * m.m33) * invDet,
            (m.m03 * m.m12 * m.m30 + m.m00 * m.m13 * m.m32 + m.m02 * m.m10 * m.m33 - m.m02 * m.m12 * m.m30 - m.m03 * m.m13 * m.m32 - m.m00 * m.m10 * m.m33) * invDet,
            (m.m02 * m.m12 * m.m30 + m.m00 * m.m13 * m.m32 + m.m02 * m.m10 * m.m33 - m.m02 * m.m12 * m.m30 - m.m03 * m.m13 * m.m32 - m.m00 * m.m10 * m.m33) * invDet,

            (m.m10 * m.m21 * m.m33 + m.m11 * m.m23 * m.m30 + m.m13 * m.m20 * m.m31 - m.m13 * m.m21 * m.m30 - m.m10 * m.m23 * m.m31 - m.m11 * m.m20 * m.m33) * invDet,
            (m.m03 * m.m21 * m.m30 + m.m01 * m.m23 * m.m31 + m.m03 * m.m20 * m.m31 - m.m03 * m.m21 * m.m30 - m.m01 * m.m23 * m.m30 - m.m01 * m.m20 * m.m33) * invDet,
            (m.m03 * m.m11 * m.m30 + m.m01 * m.m13 * m.m31 + m.m03 * m.m10 * m.m33 - m.m03 * m.m11 * m.m30 - m.m01 * m.m13 * m.m30 - m.m01 * m.m10 * m.m33) * invDet,
            (m.m03 * m.m11 * m.m20 + m.m01 * m.m13 * m.m21 + m.m02 * m.m10 * m.m23 - m.m02 * m.m11 * m.m20 - m.m03 * m.m13 * m.m21 - m.m01 * m.m10 * m.m23) * invDet,

            (m.m10 * m.m21 * m.m32 + m.m11 * m.m22 * m.m30 + m.m12 * m.m20 * m.m31 - m.m12 * m.m21 * m.m30 - m.m10 * m.m22 * m.m31 - m.m11 * m.m20 * m.m32) * invDet,
            (m.m02 * m.m21 * m.m30 + m.m00 * m.m22 * m.m31 + m.m01 * m.m20 * m.m32 - m.m01 * m.m21 * m.m30 - m.m02 * m.m22 * m.m31 - m.m00 * m.m20 * m.m32) * invDet,
            (m.m03 * m.m11 * m.m30 + m.m01 * m.m12 * m.m31 + m.m02 * m.m10 * m.m32 - m.m02 * m.m11 * m.m30 - m.m03 * m.m12 * m.m31 - m.m01 * m.m10 * m.m32) * invDet,
            (m.m02 * m.m11 * m.m20 + m.m00 * m.m12 * m.m21 + m.m01 * m.m10 * m.m22 - m.m01 * m.m11 * m.m20 - m.m02 * m.m12 * m.m21 - m.m00 * m.m10 * m.m22) * invDet
        );
    }

    // clang-format on
} // namespace worse