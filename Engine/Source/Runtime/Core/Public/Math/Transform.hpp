#pragma once
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"

#include <cmath>
#include <tuple>

namespace worse::math
{
    // clang-format off

    // =========================================================================
    // Quaternion operations
    // =========================================================================

    inline Vector3 rotateAxisAngle(Vector3 const& v, Vector3 const& axis, float angle)
    {
        Quaternion q = Quaternion::fromAxisAngle(axis, angle);
        Quaternion result = q * Quaternion(0.0f, v) * conjugate(q);
        return result.vector();
    }

    inline Vector3 rotationXAngle(Vector3 const& v, float const angle) { return rotateAxisAngle(v, Vector3::X(), angle); }
    inline Vector3 rotationYAngle(Vector3 const& v, float const angle) { return rotateAxisAngle(v, Vector3::Y(), angle); }
    inline Vector3 rotationZAngle(Vector3 const& v, float const angle) { return rotateAxisAngle(v, Vector3::Z(), angle); }

    inline Vector3 rotationEuler(Vector3 const& v, Vector3 const& euler)
    {
        Quaternion q = Quaternion::fromEuler(euler);
        Quaternion result = q * Quaternion(0.0f, v) * conjugate(q);
        return result.vector();
    }

    /// Linear interpolation
    inline Quaternion lerp(Quaternion const& q0, Quaternion const& q1, float const t)
    {
        return q0 * (1.0f - t) + q1 * t;
    }

    /// Normalized linear interpolation
    inline Quaternion nLerp(Quaternion const& q0, Quaternion const& q1, float const t)
    {
        Quaternion q = lerp(q0, q1, t);
        q = normalize(q);
        return q;
    }

    /// Spherical linear interpolation
    /// Slerp will fallback to Nlerp when quaternions are close enough
    inline Quaternion sLerp(Quaternion const& q0, Quaternion const& q1, float const t)
    {
        WS_ASSERT_MATH(isNormalized(q0), "Quat q0 not normalized");
        WS_ASSERT_MATH(isNormalized(q1), "Quat q1 not normalized");

        const float threshold = 0.9995f;
        float dot = q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
        
        // If the dot product is negative, take the shorter path by negating one quaternion
        Quaternion q1_corrected = q1;
        if (dot < 0.0f)
        {
            q1_corrected = -q1;
            dot = -dot;
        }
        
        if (dot > threshold)
        {
            return nLerp(q0, q1_corrected, t);
        }
        
        float angle = std::acos(std::clamp(dot, 0.0f, 1.0f));
        float sinAngle = std::sin(angle);
        return (q0 * std::sin(angle * (1.0f - t)) + q1_corrected * std::sin(angle * t)) / sinAngle;
    }

    // =========================================================================
    // SRT
    // =========================================================================

    inline Matrix4 makeScale(Vector3 const& scale)
    {
        return Matrix4{
            scale.x,    0.0f,    0.0f, 0.0f,
                0.0f, scale.y,    0.0f, 0.0f,
                0.0f,    0.0f, scale.z, 0.0f,
                0.0f,    0.0f,    0.0f, 1.0f
        };
    }

    inline Matrix4 makeRotation(Quaternion const& quat)
    {
        return quat.toMat4();
    }

    inline Matrix4 makeRotation(Vector3 const& euler)
    {
        return Quaternion::fromEuler(euler).toMat4();
    }

    inline Matrix4 makeTranslation(Vector3 const& translation)
    {
        return Matrix4{
            1.0f, 0.0f, 0.0f, translation.x,
            0.0f, 1.0f, 0.0f, translation.y,
            0.0f, 0.0f, 1.0f, translation.z,
            0.0f, 0.0f, 0.0f,          1.0f
        };
    }

    inline Matrix4 makeSRT(Vector3 const& scale, Quaternion const& rotation, Vector3 const& translation)
    {
        Matrix4 mat = rotation.toMat4();
        mat.col0 *= scale.x;
        mat.col1 *= scale.y;
        mat.col2 *= scale.z;
        mat.col3 = Vector4(translation, 1.0f);
        return mat;
    }

    inline Vector3 decomposeScale(Matrix4 const& mat)
    {
        float det = determinant(mat);
        WS_ASSERT_MATH(det != 0.0f, "Matrix is singular");

        Vector3 scale{
            length(mat.col0) * worse::math::signum(det),
            length(mat.col1),
            length(mat.col2)
        };

        WS_ASSERT_MATH(!isZero(scale), "Decomposed scale is zero, cause division by zero");
        return scale;
    }

    inline Quaternion decomposeRotation(Matrix4 const& mat)
    {
        float det = determinant(mat);
        WS_ASSERT_MATH(det != 0.0f, "Matrix is singular");

        Vector3 inv_scale = reciprocal(decomposeScale(mat));

        Matrix3 rotationMat{
            (mat.col0 * inv_scale.x).truncate(),
            (mat.col1 * inv_scale.y).truncate(),
            (mat.col2 * inv_scale.z).truncate()
        };

        return Quaternion::fromMat3(rotationMat);
    }

    inline Vector3 decomposeTranslation(Matrix4 const& mat)
    {
        return mat.col3.truncate();
    }

    /// Decompose a 4x4 matrix into scale, rotation and translation
    inline std::tuple<Vector3, Quaternion, Vector3>
    decomposeSRT(Matrix4 const& mat)
    {
        float det = determinant(mat);
        WS_ASSERT_MATH(det != 0.0f, "Matrix is singular");

        Vector3 scale{length(mat.col0) * worse::math::signum(det), length(mat.col1), length(mat.col2)};

        WS_ASSERT_MATH(!isZero(scale), "Decomposed scale is zero, cause division by zero");
        Vector3 inv_scale = reciprocal(scale);

        Quaternion rotation = Quaternion::fromMat3(Matrix3{
            (mat.col0 * inv_scale.x).truncate(),
            (mat.col1 * inv_scale.y).truncate(),
            (mat.col2 * inv_scale.z).truncate()
        });

        Vector3 translation = mat.col3.truncate();

        return {scale, rotation, translation};
    }

    // =========================================================================
    // Projection
    // =========================================================================

    /// Right-handed Perspective projection matrix
    /// Depth range from [0, 1]
    inline Matrix4 projectionPerspective(float verticalFov, float aspectRatio, float near, float far)
    {
        // Compute the scale factors for x and y directions
        float f = 1.0f / std::tanf(verticalFov * 0.5f); // Assumes verticalFov is in radians
        float a = f / aspectRatio;

        // Depth scaling and translation terms
        float b = far / (near - far);
        float c = near * b;

        return Matrix4{
               a, 0.0f,  0.0f,  0.0f,
            0.0f,    f,  0.0f,  0.0f,
            0.0f, 0.0f,     b,     c,
            0.0f, 0.0f, -1.0f,  0.0f
        };
    }

    /// Right-handed Orthographic projection matrix
    /// Depth range from [0, 1]
    inline Matrix4 projectionOrtho(float left, float right, float bottom, float top, float near, float far)
    {
        float recipW = 1.0f / (right - left);
        float recipH = 1.0f / (top - bottom);
        float a      = 2.0f * recipW;
        float b      = 2.0f * recipH;
        float c      = 1.0f / (near - far);
        float tx     = -(right + left) * recipW;
        float ty     = -(top + bottom) * recipH;
        float tz     = near * c;

        return Matrix4{
               a, 0.0f, 0.0f,   tx,
            0.0f,    b, 0.0f,   ty,
            0.0f, 0.0f,    c,   tz,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    /// Right-handed Orthographic projection matrix
    /// With x-y symmetry
    /// Depth range from [0, 1]
    inline Matrix4 projectionOrtho(float right, float top, float near, float far)
    {
        float a  = 1.0f / right;
        float b  = 1.0f / top;
        float c  = 1.0f / (near - far);
        float tz = near * c;

        return Matrix4{
               a, 0.0f, 0.0f, 0.0f,
            0.0f,    b, 0.0f, 0.0f,
            0.0f, 0.0f,    c,   tz,
               0,    0, 0.0f, 1.0f
        };
    }

    // =========================================================================
    // Camera
    // =========================================================================

    /// Right-handed look-at matrix for object orientation
    /// This is not a camera look-at matrix
    inline Matrix4 lookTo(Vector3 const& eye, Vector3 const& to, Vector3 const& up)
    {
        Vector3 F = normalize(to);
        Vector3 R = normalize(cross(F, up));
        Vector3 U = cross(R, F);

        return Matrix4{
             R.x,  R.y,  R.z, 0.0f,
             U.x,  U.y,  U.z, 0.0f,
             F.x,  F.y,  F.z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
            
    }

    /// Right-handed look-at matrix for camera orientation
    inline Matrix4 lookAt(Vector3 const& eye, Vector3 const& target, Vector3 const& up)
    {
        Vector3 F = normalize(target - eye);
        Vector3 R = normalize(cross(F, up));
        Vector3 U = normalize(cross(R, F));

        return Matrix4{
             R.x,  R.y,  R.z, -dot( R, eye),
             U.x,  U.y,  U.z, -dot( U, eye),
            -F.x, -F.y, -F.z, -dot(-F, eye),
            0.0f, 0.0f, 0.0f,          1.0f
        };
    }

    // clang-format on
} // namespace worse::math