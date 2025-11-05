#pragma once
#include "math/math.hpp"

namespace Worse
{

    class Camera
    {
        void recalcProjection();

    public:
        enum class ProjectionType
        {
            Perspective,
            Orthographic
        };

        Camera();

        // clang-format off

        Camera& setProjectionType(ProjectionType type);
        Camera& setPerspectiveParams(Float fovY, Float aspect, Float nearZ, Float farZ);
        Camera& setOrthoParams(Float left, Float right, Float bottom, Float top, Float nearZ, Float farZ);
        Camera& setFovY(Float fovY) { m_fovV = fovY; recalcProjection(); return *this; }
        Camera& setAspectRatio(Float aspect) { m_aspectRatio = aspect; recalcProjection(); return *this; }

        Camera& setPosition(Vector3 const& pos)                  { m_position = pos; return *this;}
        Camera& setOrientation(math::Quaternion const& rotation) { m_orientation = rotation; return *this; }
        
        // Getters
        Vector3 const&          getPosition() const       { return m_position; }
        math::Quaternion const& getOrientation() const    { return m_orientation; }
        ProjectionType          getProjectionType() const { return m_projectionType; }
        Matrix4                 getViewMatrix() const;
        Matrix4 const&          getProjectionMatrix() const     { return m_projection; }
        Matrix4                 getViewProjectionMatrix() const { return m_projection * getViewMatrix(); }
        
        // Perspective parameter getters
        Float getFovY() const        { return m_fovV; }
        Float getAspectRatio() const { return m_aspectRatio; }
        Float getNearPlane() const   { return m_nearPlane; }
        Float getFarPlane() const    { return m_farPlane; }
        
        // Orthographic parameter getters
        Float getOrthoLeft() const   { return m_orthoLeft; }
        Float getOrthoRight() const  { return m_orthoRight; }
        Float getOrthoBottom() const { return m_orthoBottom; }
        Float getOrthoTop() const    { return m_orthoTop; }
        Float getOrthoNear() const   { return m_orthoNearPlane; }
        Float getOrthoFar() const    { return m_orthoFarPlane; }
        
        // Utility methods
        Vector3 getForward() const;
        Vector3 getRight() const;
        Vector3 getUp() const;

        // clang-format on

    private:
        ProjectionType m_projectionType = ProjectionType::Perspective;

        Vector3 m_position             = Vector3::ZERO;
        math::Quaternion m_orientation = math::Quaternion::IDENTITY();

        Float m_fovV        = DegreesToRadians(45.0f);
        Float m_aspectRatio = 8.0f / 6.0f;
        Float m_nearPlane   = 0.1f;
        Float m_farPlane    = 100.f;

        Float m_orthoLeft      = -10.0f;
        Float m_orthoRight     = 10.0f;
        Float m_orthoBottom    = -10.0f;
        Float m_orthoTop       = 10.0f;
        Float m_orthoNearPlane = -10.0f;
        Float m_orthoFarPlane  = 10.0f;

        Matrix4 m_projection = Matrix4::IDENTITY;
    };

} // namespace Worse