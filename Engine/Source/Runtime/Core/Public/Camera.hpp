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
        Camera& setPerspectiveParams(float fovY, float aspect, float nearZ, float farZ);
        Camera& setOrthoParams(float left, float right, float bottom, float top, float nearZ, float farZ);
        Camera& setFovY(float fovY) { m_fovV = fovY; recalcProjection(); return *this; }
        Camera& setAspectRatio(float aspect) { m_aspectRatio = aspect; recalcProjection(); return *this; }

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
        float getFovY() const        { return m_fovV; }
        float getAspectRatio() const { return m_aspectRatio; }
        float getNearPlane() const   { return m_nearPlane; }
        float getFarPlane() const    { return m_farPlane; }
        
        // Orthographic parameter getters
        float getOrthoLeft() const   { return m_orthoLeft; }
        float getOrthoRight() const  { return m_orthoRight; }
        float getOrthoBottom() const { return m_orthoBottom; }
        float getOrthoTop() const    { return m_orthoTop; }
        float getOrthoNear() const   { return m_orthoNearPlane; }
        float getOrthoFar() const    { return m_orthoFarPlane; }
        
        // Utility methods
        math::Vector3 getForward() const;
        math::Vector3 getRight() const;
        math::Vector3 getUp() const;

        // clang-format on

    private:
        ProjectionType m_projectionType = ProjectionType::Perspective;

        math::Vector3 m_position       = math::Vector3::ZERO();
        math::Quaternion m_orientation = math::Quaternion::IDENTITY();

        float m_fovV        = math::toRadians(45.f);
        float m_aspectRatio = 8.0f / 6.0f;
        float m_nearPlane   = 0.1f;
        float m_farPlane    = 100.f;

        float m_orthoLeft      = -10.0f;
        float m_orthoRight     = 10.0f;
        float m_orthoBottom    = -10.0f;
        float m_orthoTop       = 10.0f;
        float m_orthoNearPlane = -10.0f;
        float m_orthoFarPlane  = 10.0f;

        math::Matrix4 m_projection = math::Matrix4::IDENTITY();
    };

} // namespace worse