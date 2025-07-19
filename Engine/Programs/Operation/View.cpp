#include "Renderer.hpp"
#include "../Application/World.hpp"

void World::resetCameraView(Camera* camera)
{
    if (!camera)
    {
        return;
    }
    camera->setPosition(defaultCameraPosition);
    cameraOrientation = math::Quaternion::IDENTITY();
    // camera->setOrientation(math::Quaternion::IDENTITY());
    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}

void World::setCameraToPerspectiveView(Camera* camera)
{
    if (!camera)
    {
        return;
    }
    // 透视视角：从斜上方观察点云
    math::Vector3 position = pointCloudCenter + math::Vector3{3.0f, 2.0f, 3.0f};
    math::Vector3 target   = pointCloudCenter;

    // 计算朝向目标的方向
    math::Vector3 forward = math::normalize(target - position);
    math::Vector3 up      = math::Vector3::Y();
    math::Vector3 right   = math::normalize(math::cross(forward, up));
    up                    = math::normalize(math::cross(right, forward));

    // 从forward和up向量构建四元数
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

void World::setCameraToOrthographicView(Camera* camera)
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

void World::setCameraToTopView(Camera* camera)
{
    if (!camera)
    {
        return;
    }

    camera->setPosition(downwardCameraPosition);
    cameraOrientation =
        math::Quaternion::fromEuler({math::toRadians(-90.0f), 0, 0});
    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}

void World::setCameraToSideView(Camera* camera)
{
    if (!camera)
    {
        return;
    }

    camera->setPosition(sideCameraPosition);
    cameraOrientation =
        math::Quaternion::fromEuler({0, math::toRadians(90.0f), 0});
    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());
}

void World::fitCameraToPointCloud(Camera* camera)
{
    if (!camera)
    {
        return;
    }

    // 对于变换后的点云，设置合理的观察距离
    // 点云现在应该在原点附近，包围半径约为0.5
    float distance = cloudBoundingRadius * 4.0f; // 给予足够的观察距离

    // 设置相机位置：稍微向后和向上，以便观察整个点云
    math::Vector3 cameraPosition =
        pointCloudCenter + math::Vector3{0.0f, 0.5f, distance};

    // 简单地设置相机位置，保持默认朝向（-Z方向）
    camera->setPosition(cameraPosition);

    // 如果需要，可以设置相机朝向点云中心
    // 这里暂时使用默认朝向，可以通过UI控制调整

    Renderer::setCameraPosition(camera->getPosition());
    Renderer::setCameraForward(camera->getForward());

    WS_LOG_INFO("Camera",
                "Camera fitted to point cloud - Position: ({:.3f}, {:.3f}, "
                "{:.3f}), distance: {:.3f}",
                cameraPosition.x,
                cameraPosition.y,
                cameraPosition.z,
                distance);
}