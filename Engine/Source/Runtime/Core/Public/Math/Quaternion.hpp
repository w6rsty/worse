#pragma once
#include "Base.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"

namespace worse::math
{
    // clang-format off

    struct Quaternion
    {
        union
        {
            struct { float w, x, y, z; };
            struct
            {
                float s;
                Vector3 v3;
            };
            Vector4 v4; // to simplify internal operation
        };

        constexpr Quaternion()
            : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
        constexpr Quaternion(float w, float x, float y, float z)
            : w(w), x(x), y(y), z(z) {}
        constexpr Quaternion(float s, Vector3 const& v)
            : s(s), v3(v) {}
        /// Notice the order of the elements is different from Vec4
        constexpr explicit Quaternion(Vector4 const& v)
            : v4(v) {}
        Quaternion(float* raw) : v4(raw) {}

        float scalar() const   { return s; }
        Vector3 vector() const { return v3; }

        Quaternion operator*(Quaternion const& rhs) const
        {
            return Quaternion{w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z,
                              w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
                              w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
                              w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w};
        }
        Quaternion& operator*=(Quaternion const& rhs)
        {
            *this = *this * rhs;
            return *this;
        }
        // Convert to 3x3 rotation matrix
        Matrix3 toMat3() const
        {
            float x2 = x + x;
            float y2 = y + y;
            float z2 = z + z;
            float xx = x * x2;
            float xy = x * y2;
            float xz = x * z2;
            float yy = y * y2;
            float yz = y * z2;
            float zz = z * z2;
            float wx = w * x2;
            float wy = w * y2;
            float wz = w * z2;

            return Matrix3{
                Vector3(1.0f - (yy + zz),          xy + wz,          xz - wy),
                Vector3(         xy - wz, 1.0f - (xx + zz),          yz + wx),
                Vector3(         xz + wy,          yz - wx, 1.0f - (xx + yy)),
            };
        }
        /// Convert to 4x4 rotation matrix
        Matrix4 toMat4() const { return Matrix4{toMat3()}; }
        Vector3 toEuler() const
        {
            // Convert quaternion to Euler angles (in radians)
            float sinr_cosp = 2.0f * (w * x + y * z);
            float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
            float roll      = std::atan2(sinr_cosp, cosr_cosp);

            float sinp = 2.0f * (w * y - z * x);
            float pitch = std::abs(sinp) >= 1.0f ? std::copysign(M_PI / 2, sinp) : std::asin(sinp);

            float siny_cosp = 2.0f * (w * z + x * y);
            float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
            float yaw       = std::atan2(siny_cosp, cosy_cosp);

            return Vector3(roll, pitch, yaw);
        }

        Quaternion  operator+(Quaternion const& rhs) const { return Quaternion(v4 + rhs.v4); }
        Quaternion  operator-(Quaternion const& rhs) const { return Quaternion(v4 - rhs.v4); }
        Quaternion  operator*(float const rhs) const       { return Quaternion(v4 * rhs); }
        Quaternion  operator/(float const rhs) const       { return Quaternion(v4 / rhs); }
        Quaternion  operator-() const                      { return Quaternion(-v4); }
        Quaternion& operator+=(Quaternion const& rhs)      { v4 += rhs.v4; return *this; }
        Quaternion& operator-=(Quaternion const& rhs)      { v4 -= rhs.v4; return *this; }
        Quaternion& operator*=(float const rhs)            { v4 *= rhs; return *this; }
        Quaternion& operator/=(float const rhs)            { v4 /= rhs; return *this; }

        /// From 3x3 rotation matrix
        static Quaternion fromMat3(Matrix3 const& mat)
        {
            /// Copy from glam-0.27.0/src/float/sse2/quat.rs
            auto [m00, m01, m02] = mat.col0.data;
            auto [m10, m11, m12] = mat.col1.data;
            auto [m20, m21, m22] = mat.col2.data;
            if (m22 <= 0.0f)
            {
                float dif10 = m11 - m00;
                float omm22 = 1.0f - m22;
                if (dif10 <= 0.0f)
                {
                    float four_xsq = omm22 - dif10;
                    float inv4x    = 0.5f / std::sqrt(four_xsq);
                    return Quaternion{(m12 - m21) * inv4x, four_xsq * inv4x, (m01 + m10) * inv4x, (m02 + m20) * inv4x};
                }
                else
                {
                    float four_ysq = omm22 + dif10;
                    float inv4y    = 0.5f / std::sqrt(four_ysq);
                    return Quaternion{(m20 - m02) * inv4y, (m01 + m10) * inv4y, four_ysq * inv4y, (m12 + m21) * inv4y};
                }
            }
            else
            {
                float sum10 = m11 + m00;
                float opm22 = 1.0f + m22;
                if (sum10 <= 0.0f)
                {
                    float four_zsq = opm22 - sum10;
                    float inv4z    = 0.5f / std::sqrt(four_zsq);
                    return Quaternion{(m01 - m10) * inv4z, (m02 + m20) * inv4z, (m12 + m21) * inv4z, four_zsq * inv4z};
                }
                else
                {
                    float four_wsq = opm22 + sum10;
                    float inv4w    = 0.5f / std::sqrt(four_wsq);
                    return Quaternion{four_wsq * inv4w, (m12 - m21) * inv4w, (m20 - m02) * inv4w, (m01 - m10) * inv4w};
                }
            }
        }
        /// From 4x4 rotation matrix
        static Quaternion fromMat4(Matrix4 const& mat) { return fromMat3(mat.toMat3()); }
        static Quaternion fromAxisAngle(Vector3 const& axis, float angle)
        {
            float halfAngle = angle * 0.5f;
            return Quaternion(std::cos(halfAngle), normalize(axis) * std::sin(halfAngle));
        }
        static Quaternion fromEuler(Vector3 const& euler)
        {
            Quaternion qX = fromAxisAngle(Vector3::X(), euler.x);
            Quaternion qY = fromAxisAngle(Vector3::Y(), euler.y);
            Quaternion qZ = fromAxisAngle(Vector3::Z(), euler.z);
            return qZ * qY * qX;
        }
        
        static constexpr Quaternion IDENTITY() { return Quaternion(1.0f, 0.0f, 0.0f, 0.0f); }
        static constexpr Quaternion ZERO()     { return Quaternion(0.0f, 0.0f, 0.0f, 0.0f); }
    };
    
    inline float magnitudeSquared(Quaternion const& q) { return lengthSquared(q.v4); }
    inline float magnitude(Quaternion const& q) { return length(q.v4); }
    inline Quaternion normalize(Quaternion const& q) { return Quaternion(normalize(q.v4)); }
    inline Quaternion conjugate(Quaternion const& q) { return Quaternion(q.s, -q.v3); }
    inline Quaternion inverse(Quaternion const& q)
    {
        Quaternion ret = conjugate(q);
        ret.v4 /= magnitudeSquared(ret);
        return ret;
    }
    inline bool isZero(Quaternion const& q) { return ::worse::math::isZero(q.v4); }
    inline bool isIdentity(Quaternion const& q) { return equal(q.s, 1.0f) && ::worse::math::isZero(q.v3); }
    inline bool isNormalized(Quaternion const& q) { return std::abs(magnitudeSquared(q) - 1.0f) < 1e-5f; }

    // clang-format on
} // namespace worse::math