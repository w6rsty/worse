#include "Renderer.hpp"
#include "View.hpp"
#include "../Application/World.hpp"

void setPerspectiveView(Camera* camera)
{
    camera->setProjectionType(Camera::ProjectionType::Perspective);
    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}

void setOrthographicView(Camera* camera, float scale)
{
    float const width  = scale;
    float const height = width / camera->getAspectRatio();

    camera->setOrthoParams(
        -width,
        width,
        -height,
        height,
        0.1f,
        10000.0f);
    camera->setProjectionType(Camera::ProjectionType::Orthographic);
    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}

void setTopView(Camera* camera, CameraData* cameraData, float scale)
{
    float const fovH     = camera->getFovY() / camera->getAspectRatio() * 0.5f;
    float const distance = scale * 0.5f / std::tan(fovH);

    camera->setPosition(math::Vector3{0.0f, distance, 0.0f});
    cameraData->orientation = math::Quaternion::fromEuler({math::toRadians(-90.0f), 0, 0});
    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}

void setSideView(Camera* camera, CameraData* cameraData, float scale)
{
    float const fovH     = camera->getFovY() / camera->getAspectRatio() * 0.5f;
    float const distance = scale * 0.5f / std::tan(fovH);

    camera->setPosition(math::Vector3{distance, 0.0f, 0.0f});
    cameraData->orientation = math::Quaternion::fromEuler({0, math::toRadians(90.0f), 0});
    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}

void fitView(Camera* camera, CameraData* cameraData, math::Vector3 const& focus, float scale)
{
    // 将相机垂直 Fov 转换为水平
    float const fovH     = camera->getFovY() / camera->getAspectRatio() * 0.5f;
    float const distance = scale * 0.5f / std::tan(fovH);
    WS_ASSERT(distance > 0.0f); // distance 为 0 会导致 forward 在标准化时除以 0

    math::Vector3 const position = focus + math::Vector3{distance, distance, distance};
    math::Vector3 const target   = focus;

    // 计算朝向目标的方向
    math::Vector3 const forward = math::normalize(target - position);
    math::Vector3 up            = math::Vector3::Y();
    math::Vector3 const right   = math::normalize(math::cross(forward, up));
    up                          = math::normalize(math::cross(right, forward));

    // 从 forward 和 up 向量构建四元数
    math::Matrix3 const rotMat = math::Matrix3{
        right,
        up,
        -forward // 相机的前方是-Z方向
    };

    camera->setPosition(position);
    cameraData->orientation = math::Quaternion::fromMat3(rotMat);

    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}