#pragma once
#include "Math/MathIncludes.hpp"

namespace worse
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
        Camera& setPerspectiveParams(F32 fovY, F32 aspect, F32 nearZ, F32 farZ);
        Camera& setOrthoParams(F32 left, F32 right, F32 bottom, F32 top, F32 nearZ, F32 farZ);
        Camera& setFovY(F32 fovY) { m_fovV = fovY; recalcProjection(); return *this; }
        Camera& setAspectRatio(F32 aspect) { m_aspectRatio = aspect; recalcProjection(); return *this; }

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
        F32 getFovY() const        { return m_fovV; }
        F32 getAspectRatio() const { return m_aspectRatio; }
        F32 getNearPlane() const   { return m_nearPlane; }
        F32 getFarPlane() const    { return m_farPlane; }
        
        // Orthographic parameter getters
        F32 getOrthoLeft() const   { return m_orthoLeft; }
        F32 getOrthoRight() const  { return m_orthoRight; }
        F32 getOrthoBottom() const { return m_orthoBottom; }
        F32 getOrthoTop() const    { return m_orthoTop; }
        F32 getOrthoNear() const   { return m_orthoNearPlane; }
        F32 getOrthoFar() const    { return m_orthoFarPlane; }
        
        // Utility methods
        Vector3 getForward() const;
        Vector3 getRight() const;
        Vector3 getUp() const;

        // clang-format on

    private:
        ProjectionType m_projectionType = ProjectionType::Perspective;

        Vector3 m_position             = Vector3::ZERO;
        math::Quaternion m_orientation = math::Quaternion::IDENTITY();

        F32 m_fovV        = DegreesToRadians(45.0f);
        F32 m_aspectRatio = 8.0f / 6.0f;
        F32 m_nearPlane   = 0.1f;
        F32 m_farPlane    = 100.f;

        F32 m_orthoLeft      = -10.0f;
        F32 m_orthoRight     = 10.0f;
        F32 m_orthoBottom    = -10.0f;
        F32 m_orthoTop       = 10.0f;
        F32 m_orthoNearPlane = -10.0f;
        F32 m_orthoFarPlane  = 10.0f;

        Matrix4 m_projection = Matrix4::IDENTITY;
    };

} // namespace worse