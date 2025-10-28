#pragma once
#include "Base.hpp"
#include "Vector.hpp"

namespace worse::math
{
    // clang-format off

    /// 2x2 Column major matrix
    struct Matrix2
    {
        union
        {
            struct
            {
                f32 m00, m10;
                f32 m01, m11;
            };
            struct { Vector2 col0, col1; };
            f32 data[4];
        };

        constexpr Matrix2()
            : m00(0.0f), m10(0.0f)
            , m01(0.0f), m11(0.0f) {}

        constexpr Matrix2(f32 m00, f32 m01,
                          f32 m10, f32 m11)
            : m00(m00), m01(m01)
            , m10(m10), m11(m11) {}

        constexpr Matrix2(Vector2 const& col0, Vector2 const& col1)
            : col0(col0), col1(col1) {}

        Matrix2(f32* col[2])
            : m00(col[0][0]), m01(col[1][0])
            , m10(col[0][1]), m11(col[1][1]) {}

        Matrix2(f32* raw) { std::memcpy(data, raw, sizeof(f32) * 4); }

        Matrix2 operator+(Matrix2 const& m) const { return Matrix2(col0 + m.col0, col1 + m.col1); }
        Matrix2 operator-(Matrix2 const& m) const { return Matrix2(col0 - m.col0, col1 - m.col1); }
        Matrix2 operator*(f32 const s) const    { return Matrix2(col0 * s, col1 * s); }
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
        f32& operator[](usize const index) { return data[index]; }

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
                f32 m00, m10, m20;
                f32 m01, m11, m21;
                f32 m02, m12, m22;
            };
            struct { Vector3 col0, col1, col2; };
            f32 data[9];
        };

        constexpr Matrix3()
            : m00(0.0f), m10(0.0f), m20(0.0f)
            , m01(0.0f), m11(0.0f), m21(0.0f)
            , m02(0.0f), m12(0.0f), m22(0.0f) {}

        constexpr Matrix3(f32 m00, f32 m01, f32 m02,
                          f32 m10, f32 m11, f32 m12,
                          f32 m20, f32 m21, f32 m22)
            : m00(m00), m01(m01), m02(m02)
            , m10(m10), m11(m11), m12(m12)
            , m20(m20), m21(m21), m22(m22) {}

        constexpr Matrix3(Vector3 const& col0, Vector3 const& col1, Vector3 const& col2)
            : col0(col0), col1(col1), col2(col2) {}

        Matrix3(f32* col[3])
            : m00(col[0][0]), m01(col[1][0]), m02(col[2][0])
            , m10(col[0][1]), m11(col[1][1]), m12(col[2][1])
            , m20(col[0][2]), m21(col[1][2]), m22(col[2][2]) {}

        Matrix3(f32* raw)
        {
            std::memcpy(data, raw, sizeof(f32) * 9);
        }

        Matrix3 operator+(Matrix3 const& m) const { return Matrix3(col0 + m.col0, col1 + m.col1, col2 + m.col2); }
        Matrix3 operator-(Matrix3 const& m) const { return Matrix3(col0 - m.col0, col1 - m.col1, col2 - m.col2); }
        Matrix3 operator*(f32 const s) const    { return Matrix3(col0 * s, col1 * s, col2 * s); }
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
        f32& operator[](usize const index) { return data[index]; }

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
                f32 m00, m10, m20, m30;
                f32 m01, m11, m21, m31;
                f32 m02, m12, m22, m32;
                f32 m03, m13, m23, m33;
            };
            struct { Vector4 col0, col1, col2, col3; };
            f32 data[16];
        };

        constexpr Matrix4()
            : m00(0.0f), m10(0.0f), m20(0.0f), m30(0.0f)
            , m01(0.0f), m11(0.0f), m21(0.0f), m31(0.0f)
            , m02(0.0f), m12(0.0f), m22(0.0f), m32(0.0f)
            , m03(0.0f), m13(0.0f), m23(0.0f), m33(1.0f) {}

        constexpr Matrix4(f32 m00, f32 m01, f32 m02, f32 m03,
                          f32 m10, f32 m11, f32 m12, f32 m13,
                          f32 m20, f32 m21, f32 m22, f32 m23,
                          f32 m30, f32 m31, f32 m32, f32 m33)
            : m00(m00), m01(m01), m02(m02), m03(m03)
            , m10(m10), m11(m11), m12(m12), m13(m13)
            , m20(m20), m21(m21), m22(m22), m23(m23)
            , m30(m30), m31(m31), m32(m32), m33(m33) {}

        constexpr Matrix4(Vector4 const& col0, Vector4 const& col1, Vector4 const& col2, Vector4 const& col3)
            : col0(col0), col1(col1), col2(col2), col3(col3) {}

        Matrix4(f32* col[4])
            : m00(col[0][0]), m01(col[1][0]), m02(col[2][0]), m03(col[3][0]),
              m10(col[0][1]), m11(col[1][1]), m12(col[2][1]), m13(col[3][1]),
              m20(col[0][2]), m21(col[1][2]), m22(col[2][2]), m23(col[3][2]),
              m30(col[0][3]), m31(col[1][3]), m32(col[2][3]), m33(col[3][3]) {}

        Matrix4(f32* raw) { std::memcpy(data, raw, sizeof(f32) * 16); }

        Matrix4(Matrix3 mat3)
            : m00(mat3.m00), m01(mat3.m01), m02(mat3.m02), m03(0)
            , m10(mat3.m10), m11(mat3.m11), m12(mat3.m12), m13(0)
            , m20(mat3.m20), m21(mat3.m21), m22(mat3.m22), m23(0)
            , m30(0),        m31(0),        m32(0),        m33(1) {}

        Matrix3 toMat3() const { return Matrix3(m00, m01, m02, m10, m11, m12, m20, m21, m22); }

        Matrix4 operator+(Matrix4 const& m) const { return Matrix4(col0 + m.col0, col1 + m.col1, col2 + m.col2, col3 + m.col3); }
        Matrix4 operator-(Matrix4 const& m) const { return Matrix4(col0 - m.col0, col1 - m.col1, col2 - m.col2, col3 - m.col3); }
        Matrix4 operator*(f32 const s) const    { return Matrix4(col0 * s, col1 * s, col2 * s, col3 * s); }
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
        f32& operator[](usize const index) { return data[index]; }

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

    inline constexpr f32 determinant(Matrix2 const& m)
    {
        return
            m.m00 * m.m11 -
            m.m01 * m.m10;
    }
    inline constexpr f32 determinant(Matrix3 const& m)
    {
        return
            m.m00 * ( m.m11 *  m.m22 -  m.m12 *  m.m21) - 
            m.m01 * ( m.m10 *  m.m22 -  m.m12 *  m.m20) +
            m.m02 * ( m.m10 *  m.m21 -  m.m11 *  m.m20);

    }
    inline constexpr f32 determinant(Matrix4 const& m)
    {
        return
            m.m00 * (m.m11 * m.m22 * m.m33 + m.m12 * m.m23 * m.m31 + m.m13 * m.m21 * m.m32 - m.m13 * m.m22 * m.m31 - m.m11 * m.m23 * m.m32 - m.m12 * m.m21 * m.m33) -    
            m.m01 * (m.m10 * m.m22 * m.m33 + m.m12 * m.m23 * m.m30 + m.m13 * m.m20 * m.m32 - m.m13 * m.m22 * m.m30 - m.m10 * m.m23 * m.m32 - m.m12 * m.m20 * m.m33) +
            m.m02 * (m.m10 * m.m21 * m.m33 + m.m11 * m.m23 * m.m30 + m.m13 * m.m20 * m.m31 - m.m13 * m.m21 * m.m30 - m.m10 * m.m23 * m.m31 - m.m11 * m.m20 * m.m33) -
            m.m03 * (m.m10 * m.m21 * m.m32 + m.m11 * m.m22 * m.m30 + m.m12 * m.m20 * m.m31 - m.m12 * m.m21 * m.m30 - m.m10 * m.m22 * m.m31 - m.m11 * m.m20 * m.m32);
    }

    inline constexpr f32 trace(Matrix2 const& m) { return m.m00 + m.m11; }
    inline constexpr f32 trace(Matrix3 const& m) { return m.m00 + m.m11 + m.m22; }
    inline constexpr f32 trace(Matrix4 const& m) { return m.m00 + m.m11 + m.m22 + m.m33; }

    inline constexpr Matrix2 inverse(Matrix2 const& m)
    {
        f32 det = determinant(m);
        if (det == 0)
        {
            return Matrix2::NANM();
        }
        f32 invDet = 1.0f / det;
        return Matrix2(
             m.m11 * invDet, -m.m01 * invDet,
            -m.m10 * invDet,  m.m00 * invDet);
    }
    inline constexpr Matrix3 inverse(Matrix3 const& m)
    {
        f32 det = determinant(m);
        if (det == 0)
        {
            return Matrix3::NANM();
        }
        f32 invDet = 1.0f / det;
        return Matrix3(
            (m.m11 * m.m22 - m.m12 * m.m21) * invDet, // C00
            (m.m12 * m.m20 - m.m10 * m.m22) * invDet, // C01
            (m.m10 * m.m21 - m.m11 * m.m20) * invDet, // C02

            (m.m02 * m.m21 - m.m01 * m.m22) * invDet, // C10
            (m.m00 * m.m22 - m.m02 * m.m20) * invDet, // C11
            (m.m01 * m.m20 - m.m00 * m.m21) * invDet, // C12

            (m.m01 * m.m12 - m.m02 * m.m11) * invDet, // C20
            (m.m02 * m.m10 - m.m00 * m.m12) * invDet, // C21
            (m.m00 * m.m11 - m.m01 * m.m10) * invDet  // C22
        );
    }
    inline constexpr Matrix4 inverse(Matrix4 const& m)
    {
        // 使用Cramer法则（伴随矩阵法）求逆
        // 为了提高可读性和减少错误，我们先创建一些临时变量
        f32 const m00 = m.m00, m01 = m.m01, m02 = m.m02, m03 = m.m03;
        f32 const m10 = m.m10, m11 = m.m11, m12 = m.m12, m13 = m.m13;
        f32 const m20 = m.m20, m21 = m.m21, m22 = m.m22, m23 = m.m23;
        f32 const m30 = m.m30, m31 = m.m31, m32 = m.m32, m33 = m.m33;

        // 计算第一行和第二行元素构成的 2x2 子行列式，用于后续计算
        f32 const s0 = m00 * m11 - m10 * m01;
        f32 const s1 = m00 * m12 - m10 * m02;
        f32 const s2 = m00 * m13 - m10 * m03;
        f32 const s3 = m01 * m12 - m11 * m02;
        f32 const s4 = m01 * m13 - m11 * m03;
        f32 const s5 = m02 * m13 - m12 * m03;

        // 计算第三行和第四行元素构成的 2x2 子行列式
        f32 const c0 = m20 * m31 - m30 * m21;
        f32 const c1 = m20 * m32 - m30 * m22;
        f32 const c2 = m20 * m33 - m30 * m23;
        f32 const c3 = m21 * m32 - m31 * m22;
        f32 const c4 = m21 * m33 - m31 * m23;
        f32 const c5 = m22 * m33 - m32 * m23;

        // 利用拉普拉斯展开计算 4x4 行列式
        // det = s0*c5 - s1*c4 + s2*c3 + s3*c2 - s4*c1 + s5*c0
        f32 const det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;

        if (det == 0.0f)
        {
            return Matrix4::NANM(); // 矩阵不可逆
        }

        f32 const invDet = 1.0f / det;
        
        // 计算伴随矩阵（辅因子矩阵的转置）的每个元素，并乘以 1/det
        // 直接计算出逆矩阵的每个元素
        return Matrix4(
            ( m11 * c5 - m12 * c4 + m13 * c3) * invDet,
            (-m01 * c5 + m02 * c4 - m03 * c3) * invDet,
            ( m31 * s5 - m32 * s4 + m33 * s3) * invDet,
            (-m21 * s5 + m22 * s4 - m23 * s3) * invDet,

            (-m10 * c5 + m12 * c2 - m13 * c1) * invDet,
            ( m00 * c5 - m02 * c2 + m03 * c1) * invDet,
            (-m30 * s5 + m32 * s2 - m33 * s1) * invDet,
            ( m20 * s5 - m22 * s2 + m23 * s1) * invDet,

            ( m10 * c4 - m11 * c2 + m13 * c0) * invDet,
            (-m00 * c4 + m01 * c2 - m03 * c0) * invDet,
            ( m30 * s4 - m31 * s2 + m33 * s0) * invDet,
            (-m20 * s4 + m21 * s2 - m23 * s0) * invDet,

            (-m10 * c3 + m11 * c1 - m12 * c0) * invDet,
            ( m00 * c3 - m01 * c1 + m02 * c0) * invDet,
            (-m30 * s3 + m31 * s1 - m32 * s0) * invDet,
            ( m20 * s3 - m21 * s1 + m22 * s0) * invDet
        );
    }
    // clang-format on
} // namespace worse::math