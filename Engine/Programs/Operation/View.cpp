#include "Renderer.hpp"
#include "../Application/World.hpp"

void World::setCameraPerspectiveView(Camera* camera, math::Vector3 const& focus, float scale)
{
    if (!camera)
    {
        return;
    }

    // 将相机垂直 Fov 转换为水平
    float fovH     = camera->getFovY() / camera->getAspectRatio() * 0.5f;
    float distance = scale * 0.5f / std::tan(fovH);

    // 透视
    math::Vector3 position = focus + math::Vector3{distance, distance, distance};
    math::Vector3 target   = focus;

    // 计算朝向目标的方向
    math::Vector3 forward = math::normalize(target - position);
    math::Vector3 up      = math::Vector3::Y();
    math::Vector3 right   = math::normalize(math::cross(forward, up));
    up                    = math::normalize(math::cross(right, forward));

    // 从 forward 和 up 向量构建四元数
    math::Matrix3 rotMat = math::Matrix3{
        right,
        up,
        -forward // 相机的前方是-Z方向
    };

    camera->setPosition(position);
    cameraOrientation = math::Quaternion::fromMat3(rotMat);
    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}

void World::setCameraOrthographicView(Camera* camera)
{
    if (!camera)
    {
        return;
    }

    camera->setPosition(defaultCameraPosition);
    cameraOrientation = math::Quaternion::IDENTITY();
    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}

void World::setCameraTopView(Camera* camera)
{
    if (!camera)
    {
        return;
    }

    camera->setPosition(downwardCameraPosition);
    cameraOrientation = math::Quaternion::fromEuler({math::toRadians(-90.0f), 0, 0});
    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}

void World::setCameraSideView(Camera* camera)
{
    if (!camera)
    {
        return;
    }

    camera->setPosition(sideCameraPosition);
    cameraOrientation = math::Quaternion::fromEuler({0, math::toRadians(90.0f), 0});
    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}

void World::fitCameraCloud(Camera* camera, math::Vector3 const& focus, float scale)
{
    if (!camera)
    {
        return;
    }

    float fovH     = camera->getFovY() / camera->getAspectRatio() * 0.5f;
    float distance = scale * 0.5f / std::tan(fovH);

    math::Vector3 cameraPosition = focus + math::Vector3{0.0f, 0.0f, distance};

    camera->setPosition(cameraPosition);
    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}