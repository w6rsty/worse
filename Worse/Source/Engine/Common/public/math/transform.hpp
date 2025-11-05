#pragma once
#include "common_macro.hpp"
#include "math/math.hpp"
#include "math/vector.hpp"
#include "math/matrix.hpp"
#include "math/quaternion.hpp"

#include <cmath>
#include <tuple>

namespace Worse::math
{
    // clang-format off

    // =========================================================================
    // Quaternion operations
    // =========================================================================

    inline Vector3 rotateAxisAngle(Vector3 const& v, Vector3 const& axis, Float angle)
    {
        Quaternion q = Quaternion::fromAxisAngle(axis, angle);
        Quaternion result = q * Quaternion(0.0f, v) * conjugate(q);
        return result.vector();
    }

    inline Vector3 rotationXAngle(Vector3 const& v, Float const angle) { return rotateAxisAngle(v, Vector3::UNIT_X, angle); }
    inline Vector3 rotationYAngle(Vector3 const& v, Float const angle) { return rotateAxisAngle(v, Vector3::UNIT_Y, angle); }
    inline Vector3 rotationZAngle(Vector3 const& v, Float const angle) { return rotateAxisAngle(v, Vector3::UNIT_Z, angle); }

    inline Vector3 rotationEuler(Vector3 const& v, Vector3 const& euler)
    {
        Quaternion q = Quaternion::fromEuler(euler);
        Quaternion result = q * Quaternion(0.0f, v) * conjugate(q);
        return result.vector();
    }

    /// Linear interpolation
    inline Quaternion lerp(Quaternion const& q0, Quaternion const& q1, Float const t)
    {
        return q0 * (1.0f - t) + q1 * t;
    }

    /// Normalized linear interpolation
    inline Quaternion nLerp(Quaternion const& q0, Quaternion const& q1, Float const t)
    {
        Quaternion q = lerp(q0, q1, t);
        q = Normalize(q);
        return q;
    }

    /// Spherical linear interpolation
    /// Slerp will fallback to Nlerp when quaternions are close enough
    inline Quaternion sLerp(Quaternion const& q0, Quaternion const& q1, Float const t)
    {
        WORSE_ASSERT_MSG(isNormalized(q0), "Quat q0 not normalized");
        WORSE_ASSERT_MSG(isNormalized(q1), "Quat q1 not normalized");

        const Float threshold = 0.9995f;
        Float dot = q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
        
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
        
        Float angle = std::acos(std::clamp(dot, 0.0f, 1.0f));
        Float sinAngle = std::sin(angle);
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
        mat[0] *= scale.x;
        mat[1] *= scale.y;
        mat[2] *= scale.z;
        mat[3] = Vector4(translation.x, translation.y, translation.z, 1.0f);
        return mat;
    }

    inline Vector3 decomposeScale(Matrix4 const& mat)
    {
        Float det = Det(mat);
        WORSE_ASSERT_MSG(det != 0.0f, "Matrix is singular");

        Vector3 scale{
            mat[0].Length() * Math::Signum(det),
            mat[1].Length(),
            mat[2].Length()
        };

        WORSE_ASSERT_MSG(scale != Vector3::ZERO, "Decomposed scale is zero, cause division by zero");
        return scale;
    }

    inline Quaternion decomposeRotation(Matrix4 const& mat)
    {
        Float det = Det(mat);
        WORSE_ASSERT_MSG(det != 0.0f, "Matrix is singular");

        Vector3 inv_scale = 1.0f / decomposeScale(mat);

        Matrix3 rotationMat{
            Vector3{(mat[0] * inv_scale.x).ptr()},
            Vector3{(mat[1] * inv_scale.y).ptr()},
            Vector3{(mat[2] * inv_scale.z).ptr()}
        };

        return Quaternion::fromMat3(rotationMat);
    }

    inline Vector3 decomposeTranslation(Matrix4 const& mat)
    {
        return Vector3{mat[3].ptr()};
    }

    /// Decompose a 4x4 matrix into scale, rotation and translation
    inline std::tuple<Vector3, Quaternion, Vector3>
    decomposeSRT(Matrix4 const& mat)
    {
        Float det = Det(mat);
        WORSE_ASSERT_MSG(det != 0.0f, "Matrix is singular");

        Vector3 scale{mat[0].Length() * Math::Signum(det), mat[1].Length(), mat[2].Length()};

        WORSE_ASSERT_MSG(scale != Vector3::ZERO, "Decomposed scale is zero, cause division by zero");
        Vector3 inv_scale = 1.0f / scale;

        Quaternion rotation = Quaternion::fromMat3(Matrix3{
            Vector3{(mat[0] * inv_scale.x).ptr()},
            Vector3{(mat[1] * inv_scale.y).ptr()},
            Vector3{(mat[2] * inv_scale.z).ptr()}
        });

        Vector3 translation{mat[3].ptr()};

        return {scale, rotation, translation};
    }

    // =========================================================================
    // Projection
    // =========================================================================

  /// Right-handed Perspective projection matrix
    /// Depth range from [0, 1] (near = 1, far = 0)
    inline Matrix4 projectionPerspective(Float verticalFov, Float aspectRatio, Float near, Float far)
    {
        // Compute the scale factors for x and y directions
        Float f = 1.0f / std::tanf(verticalFov * 0.5f); // Assumes verticalFov is in radians
        Float a = f / aspectRatio;

        // Depth scaling and translation terms
        Float b = near / (far - near);
        Float c = near * far / (far - near); 

        return Matrix4{
               a, 0.0f,  0.0f,  0.0f,
            0.0f,    f,  0.0f,  0.0f,
            0.0f, 0.0f,     b,     c,
            0.0f, 0.0f, -1.0f,  0.0f
        };
    }

    /// Right-handed Orthographic projection matrix
    /// REVERSED-Z with depth range from [1, 0] (near=1, far=0)
    inline Matrix4 projectionOrtho(Float left, Float right, Float bottom, Float top, Float near, Float far)
    {
        Float recipW = 1.0f / (right - left);
        Float recipH = 1.0f / (top - bottom);
        Float a      = 2.0f * recipW;
        Float b      = 2.0f * recipH;
        Float tx     = -(right + left) * recipW;
        Float ty     = -(top + bottom) * recipH;   
        Float c      = 1.0f / (far - near);
        Float tz     = far * c;

        return Matrix4{
            a,    0.0f, 0.0f,   tx,
            0.0f,    b, 0.0f,   ty,
            0.0f, 0.0f,    c,   tz,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }
    /// Right-handed Orthographic projection matrix
    /// With x-y symmetry
    /// REVERSED-Z with depth range from [1, 0] (near=1, far=0)
    inline Matrix4 projectionOrtho(Float right, Float top, Float near, Float far)
    {
        Float a  = 1.0f / right;
        Float b  = 1.0f / top;
        Float c  = 1.0f / (far - near);
        Float tz = far * c;

        return Matrix4{
            a,    0.0f, 0.0f, 0.0f,
            0.0f,    b, 0.0f, 0.0f,
            0.0f, 0.0f,    c,   tz,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    // =========================================================================
    // Camera
    // =========================================================================

    /// Right-handed look-at matrix for object orientation
    /// This is not a camera look-at matrix
    inline Matrix4 lookTo(Vector3 const& eye, Vector3 const& to, Vector3 const& up)
    {
        Vector3 F = Normalize(to);
        Vector3 R = Normalize(CrossProduct(F, up));
        Vector3 U = CrossProduct(R, F);

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
        Vector3 F = Normalize(target - eye);
        Vector3 R = Normalize(CrossProduct(F, up));
        Vector3 U = Normalize(CrossProduct(R, F));

        return Matrix4{
             R.x,  R.y,  R.z, -DotProduct( R, eye),
             U.x,  U.y,  U.z, -DotProduct( U, eye),
            -F.x, -F.y, -F.z, -DotProduct(-F, eye),
            0.0f, 0.0f, 0.0f,                 1.0f
        };
    }

    // clang-format on
} // namespace Worse::math