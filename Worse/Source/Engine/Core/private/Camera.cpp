#include "Camera.hpp"
#include "math/math_includes.hpp"

namespace Worse
{

    Camera::Camera()
    {
        recalcProjection();
    }

    Camera& Camera::setProjectionType(ProjectionType type)
    {
        m_projectionType = type;
        recalcProjection();
        return *this;
    }

    Camera& Camera::setPerspectiveParams(Float fovY, Float aspect, Float nearZ, Float farZ)
    {
        // Validate parameters
        if (fovY <= 0.0f || fovY >= kPi || aspect <= 0.0f || nearZ <= 0.0f || farZ <= nearZ)
        {
            return *this;
        }

        m_fovV        = fovY;
        m_aspectRatio = aspect;
        m_nearPlane   = nearZ;
        m_farPlane    = farZ;
        if (m_projectionType == ProjectionType::Perspective)
        {
            recalcProjection();
        }
        return *this;
    }

    Camera& Camera::setOrthoParams(Float left, Float right, Float bottom, Float top, Float nearZ, Float farZ)
    {
        // Validate parameters
        if (right <= left || top <= bottom || farZ <= nearZ)
        {
            return *this;
        }

        m_orthoLeft      = left;
        m_orthoRight     = right;
        m_orthoBottom    = bottom;
        m_orthoTop       = top;
        m_orthoNearPlane = nearZ;
        m_orthoFarPlane  = farZ;
        if (m_projectionType == ProjectionType::Orthographic)
        {
            recalcProjection();
        }
        return *this;
    }

    void Camera::recalcProjection()
    {
        if (m_projectionType == ProjectionType::Perspective)
        {
            m_projection = math::projectionPerspective(m_fovV,
                                                       m_aspectRatio,
                                                       m_nearPlane,
                                                       m_farPlane);
        }
        else if (m_projectionType == ProjectionType::Orthographic)
        {
            m_projection = math::projectionOrtho(m_orthoLeft,
                                                 m_orthoRight,
                                                 m_orthoBottom,
                                                 m_orthoTop,
                                                 m_orthoNearPlane,
                                                 m_orthoFarPlane);
        }
    }

    Matrix4 Camera::getViewMatrix() const
    {
        Vector3 const forward = getForward();
        Vector3 const up      = getUp();
        Vector3 const target  = m_position + forward;
        return math::lookAt(m_position, target, up);
    }

    Vector3 Camera::getForward() const
    {
        // Forward is -Z in camera space, transform by orientation
        // Use quaternion rotation: q * v * q*
        math::Quaternion vecQuat(0.0f, Vector3(0.0f, 0.0f, -1.0f));
        math::Quaternion result =
            m_orientation * vecQuat * math::conjugate(m_orientation);
        return result.vector();
    }

    Vector3 Camera::getRight() const
    {
        // Right is +X in camera space, transform by orientation
        math::Quaternion vecQuat(0.0f, Vector3(1.0f, 0.0f, 0.0f));
        math::Quaternion result =
            m_orientation * vecQuat * math::conjugate(m_orientation);
        return result.vector();
    }

    Vector3 Camera::getUp() const
    {
        // Up is +Y in camera space, transform by orientation
        math::Quaternion vecQuat(0.0f, Vector3(0.0f, 1.0f, 0.0f));
        math::Quaternion result =
            m_orientation * vecQuat * math::conjugate(m_orientation);
        return result.vector();
    }

} // namespace Worse