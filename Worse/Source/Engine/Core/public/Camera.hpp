#pragma once
#include "Math/Base.hpp"
#include "Math/Math.hpp"

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
        Camera& setPerspectiveParams(f32 fovY, f32 aspect, f32 nearZ, f32 farZ);
        Camera& setOrthoParams(f32 left, f32 right, f32 bottom, f32 top, f32 nearZ, f32 farZ);
        Camera& setFovY(f32 fovY) { m_fovV = fovY; recalcProjection(); return *this; }
        Camera& setAspectRatio(f32 aspect) { m_aspectRatio = aspect; recalcProjection(); return *this; }

        Camera& setPosition(math::Vector3 const& pos)            { m_position = pos; return *this;}
        Camera& setOrientation(math::Quaternion const& rotation) { m_orientation = rotation; return *this; }
        
        // Getters
        math::Vector3 const&    getPosition() const       { return m_position; }
        math::Quaternion const& getOrientation() const    { return m_orientation; }
        ProjectionType          getProjectionType() const { return m_projectionType; }
        math::Matrix4           getViewMatrix() const;
        math::Matrix4 const&    getProjectionMatrix() const { return m_projection; }
        math::Matrix4           getViewProjectionMatrix() const { return m_projection * getViewMatrix(); }
        
        // Perspective parameter getters
        f32 getFovY() const        { return m_fovV; }
        f32 getAspectRatio() const { return m_aspectRatio; }
        f32 getNearPlane() const   { return m_nearPlane; }
        f32 getFarPlane() const    { return m_farPlane; }
        
        // Orthographic parameter getters
        f32 getOrthoLeft() const   { return m_orthoLeft; }
        f32 getOrthoRight() const  { return m_orthoRight; }
        f32 getOrthoBottom() const { return m_orthoBottom; }
        f32 getOrthoTop() const    { return m_orthoTop; }
        f32 getOrthoNear() const   { return m_orthoNearPlane; }
        f32 getOrthoFar() const    { return m_orthoFarPlane; }
        
        // Utility methods
        math::Vector3 getForward() const;
        math::Vector3 getRight() const;
        math::Vector3 getUp() const;

        // clang-format on

    private:
        ProjectionType m_projectionType = ProjectionType::Perspective;

        math::Vector3 m_position       = math::Vector3::ZERO();
        math::Quaternion m_orientation = math::Quaternion::IDENTITY();

        f32 m_fovV        = math::toRadians(45.f);
        f32 m_aspectRatio = 8.0f / 6.0f;
        f32 m_nearPlane   = 0.1f;
        f32 m_farPlane    = 100.f;

        f32 m_orthoLeft      = -10.0f;
        f32 m_orthoRight     = 10.0f;
        f32 m_orthoBottom    = -10.0f;
        f32 m_orthoTop       = 10.0f;
        f32 m_orthoNearPlane = -10.0f;
        f32 m_orthoFarPlane  = 10.0f;

        math::Matrix4 m_projection = math::Matrix4::IDENTITY();
    };

} // namespace worse