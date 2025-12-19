#pragma once

#include "Math/Math.hpp"
#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"

namespace worse::math
{
    // clang-format off

    struct Quaternion
    {
        union
        {
            struct { F32 w, x, y, z; };
            struct
            {
                F32 s;
                Vector3 v3;
            };
            Vector4 v4; // to simplify internal operation
        };

        constexpr Quaternion()
            : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
        constexpr Quaternion(F32 w, F32 x, F32 y, F32 z)
            : w(w), x(x), y(y), z(z) {}
        constexpr Quaternion(F32 s, Vector3 const& v)
            : s(s), v3(v) {}
        /// Notice the order of the elements is different from Vec4
        constexpr explicit Quaternion(Vector4 const& v)
            : v4(v) {}
        Quaternion(F32* raw) : v4(raw) {}

        F32 scalar() const   { return s; }
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
            F32 x2 = x + x;
            F32 y2 = y + y;
            F32 z2 = z + z;
            F32 xx = x * x2;
            F32 xy = x * y2;
            F32 xz = x * z2;
            F32 yy = y * y2;
            F32 yz = y * z2;
            F32 zz = z * z2;
            F32 wx = w * x2;
            F32 wy = w * y2;
            F32 wz = w * z2;

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
            F32 sinr_cosp = 2.0f * (w * x + y * z);
            F32 cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
            F32 roll      = FMath::Atan2(sinr_cosp, cosr_cosp);

            F32 sinp = 2.0f * (w * y - z * x);
            F32 pitch = std::abs(sinp) >= 1.0f ? std::copysign(FMath::kPi / 2, sinp) : std::asin(sinp);

            F32 siny_cosp = 2.0f * (w * z + x * y);
            F32 cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
            F32 yaw       = FMath::Atan2(siny_cosp, cosy_cosp);

            return Vector3(roll, pitch, yaw);
        }

        Quaternion  operator+(Quaternion const& rhs) const { return Quaternion(v4 + rhs.v4); }
        Quaternion  operator-(Quaternion const& rhs) const { return Quaternion(v4 - rhs.v4); }
        Quaternion  operator*(F32 const rhs) const       { return Quaternion(v4 * rhs); }
        Quaternion  operator/(F32 const rhs) const       { return Quaternion(v4 / rhs); }
        Quaternion  operator-() const                      { return Quaternion(-v4); }
        Quaternion& operator+=(Quaternion const& rhs)      { v4 += rhs.v4; return *this; }
        Quaternion& operator-=(Quaternion const& rhs)      { v4 -= rhs.v4; return *this; }
        Quaternion& operator*=(F32 const rhs)            { v4 *= rhs; return *this; }
        Quaternion& operator/=(F32 const rhs)            { v4 /= rhs; return *this; }

        /// From 3x3 rotation matrix
        static Quaternion fromMat3(Matrix3 const& mat)
        {
            /// Copy from glam-0.27.0/src/f32/sse2/quat.rs
            auto [m00, m01, m02] = mat[0];
            auto [m10, m11, m12] = mat[1];
            auto [m20, m21, m22] = mat[2];
            if (m22 <= 0.0f)
            {
                F32 dif10 = m11 - m00;
                F32 omm22 = 1.0f - m22;
                if (dif10 <= 0.0f)
                {
                    F32 four_xsq = omm22 - dif10;
                    F32 inv4x    = 0.5f / std::sqrt(four_xsq);
                    return Quaternion{(m12 - m21) * inv4x, four_xsq * inv4x, (m01 + m10) * inv4x, (m02 + m20) * inv4x};
                }
                else
                {
                    F32 four_ysq = omm22 + dif10;
                    F32 inv4y    = 0.5f / std::sqrt(four_ysq);
                    return Quaternion{(m20 - m02) * inv4y, (m01 + m10) * inv4y, four_ysq * inv4y, (m12 + m21) * inv4y};
                }
            }
            else
            {
                F32 sum10 = m11 + m00;
                F32 opm22 = 1.0f + m22;
                if (sum10 <= 0.0f)
                {
                    F32 four_zsq = opm22 - sum10;
                    F32 inv4z    = 0.5f / std::sqrt(four_zsq);
                    return Quaternion{(m01 - m10) * inv4z, (m02 + m20) * inv4z, (m12 + m21) * inv4z, four_zsq * inv4z};
                }
                else
                {
                    F32 four_wsq = opm22 + sum10;
                    F32 inv4w    = 0.5f / std::sqrt(four_wsq);
                    return Quaternion{four_wsq * inv4w, (m12 - m21) * inv4w, (m20 - m02) * inv4w, (m01 - m10) * inv4w};
                }
            }
        }
        /// From 4x4 rotation matrix
        static Quaternion fromMat4(Matrix4 const& mat) { return fromMat3(static_cast<Matrix3>(mat)); }
        static Quaternion fromAxisAngle(Vector3 const& axis, F32 angle)
        {
            F32 halfAngle = angle * 0.5f;
            return Quaternion(FMath::Cos(halfAngle), Normalize(axis) * FMath::Sin(halfAngle));
        }
        static Quaternion fromEuler(Vector3 const& euler)
        {
            Quaternion qX = fromAxisAngle(Vector3::UNIT_X, euler.x);
            Quaternion qY = fromAxisAngle(Vector3::UNIT_Y, euler.y);
            Quaternion qZ = fromAxisAngle(Vector3::UNIT_Z, euler.z);
            return qZ * qY * qX;
        }
        
        static constexpr Quaternion IDENTITY() { return Quaternion(1.0f, 0.0f, 0.0f, 0.0f); }
        static constexpr Quaternion ZERO()     { return Quaternion(0.0f, 0.0f, 0.0f, 0.0f); }
    };
    
    inline F32 magnitudeSquared(Quaternion const& q) { return q.v4.LengthSquared(); }
    inline F32 magnitude(Quaternion const& q) { return q.v4.Length(); }
    inline Quaternion Normalize(Quaternion const& q) { return Quaternion(Normalize(q.v4)); }
    inline Quaternion conjugate(Quaternion const& q) { return Quaternion(q.s, -q.v3); }
    inline Quaternion inverse(Quaternion const& q)
    {
        Quaternion ret = conjugate(q);
        ret.v4 /= magnitudeSquared(ret);
        return ret;
    }
    inline bool isZero(Quaternion const& q) { return q.v4 == Vector4::ZERO; }
    inline bool isIdentity(Quaternion const& q) { return Eq(q.s, 1.0f) && (q.v3 == Vector3::ZERO); }
    inline bool isNormalized(Quaternion const& q) { return Eq(magnitudeSquared(q), 1.0f); }

    // clang-format on
} // namespace worse::math