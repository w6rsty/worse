#include "Math/Base.hpp"
#include "Math/Quaternion.hpp"
#include "Renderer.hpp"

#include "World.hpp"

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
    float distance = pointCloudBoundingRadius * 4.0f; // 给予足够的观察距离

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

// 初始化可用文件列表的函数
void World::initializeAvailableFiles()
{
    availableFiles.clear();

    WS_LOG_INFO("PointCloud",
                "Scanning point cloud directory: {}",
                POINT_CLOUD_DIRECTORY);

    // 获取所有.las文件
    for (const auto& entry :
         std::filesystem::directory_iterator(POINT_CLOUD_DIRECTORY))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".las")
        {
            std::string filename = entry.path().filename().string();
            availableFiles.push_back(filename);
        }
    }

    // 按文件名排序以获得一致的顺序
    std::sort(availableFiles.begin(), availableFiles.end());

    WS_LOG_INFO("PointCloud",
                "Found {} point cloud files",
                availableFiles.size());
}

// 运行时加载点云网格的函数
bool World::loadPointCloudMesh(const std::string& filename,
                               ecs::Commands& commands)
{
    // 检查是否已加载
    if (loadedMeshes.find(filename) != loadedMeshes.end())
    {
        WS_LOG_INFO("PointCloud", "Mesh for {} already loaded", filename);
        return true;
    }

    isLoadingMesh      = true;
    currentLoadingFile = filename;
    loadingProgress    = 0.0f;

    try
    {
        std::string fullPath = POINT_CLOUD_DIRECTORY + filename;

        WS_LOG_INFO("PointCloud", "Loading point cloud mesh: {}", filename);

        // 加载点云数据
        loadingProgress      = 0.2f;
        pc::Cloud pointCloud = pc::load(fullPath);

        if (!pointCloud.points.empty())
        {
            loadingProgress = 0.6f;

            // 创建网格并保存索引
            auto meshes = commands.getResourceArray<Mesh>();
            std::size_t meshIndex =
                meshes.add(CustomMesh3D{.vertices = pointCloud.points});

            loadingProgress = 0.8f;

            // 关键：创建GPU缓冲区
            meshes.get(meshIndex)->createGPUBuffers();

            loadingProgress = 1.0f;

            loadedMeshes[filename] = meshIndex;

            WS_LOG_INFO("PointCloud",
                        "Loaded mesh for {}: {} points, mesh index: {}",
                        filename,
                        pointCloud.points.size(),
                        meshIndex);

            isLoadingMesh      = false;
            currentLoadingFile = "";
            return true;
        }
        else
        {
            WS_LOG_WARN("PointCloud", "Empty point cloud: {}", filename);
            isLoadingMesh      = false;
            currentLoadingFile = "";
            return false;
        }
    }
    catch (const std::exception& e)
    {
        WS_LOG_ERROR("PointCloud",
                     "Failed to load mesh for {}: {}",
                     filename,
                     e.what());
        isLoadingMesh      = false;
        currentLoadingFile = "";
        return false;
    }
}

// 清理所有已加载的网格的函数
void World::clearAllLoadedMeshes(ecs::Commands& commands)
{
    WS_LOG_INFO("PointCloud", "Clearing all loaded meshes...");

    // 如果有当前活跃的点云Entity，先销毁
    if (pointCloudEntity != ecs::Entity::null())
    {
        commands.destroy(pointCloudEntity);
        pointCloudEntity = ecs::Entity::null();
        hasPointCloud    = false;
        WS_LOG_INFO("PointCloud", "Destroyed current point cloud entity");
    }

    // 获取网格资源数组
    auto meshes = commands.getResourceArray<Mesh>();

    // 遍历所有已加载的网格并释放
    for (const auto& [filename, meshIndex] : loadedMeshes)
    {
        try
        {
            // 尝试获取并清理网格
            if (auto* mesh = meshes.get(meshIndex))
            {
                mesh->clear();
                WS_LOG_INFO("PointCloud",
                            "Cleared mesh for {}: index {}",
                            filename,
                            meshIndex);
            }
        }
        catch (const std::exception& e)
        {
            WS_LOG_WARN("PointCloud",
                        "Failed to clear mesh for {}: {}",
                        filename,
                        e.what());
        }
    }

    // 清空映射表
    size_t clearedCount = loadedMeshes.size();
    loadedMeshes.clear();

    // 重置点云相关状态
    pointCloudData           = pc::Cloud{}; // 重置为空
    pointCloudCenter         = math::Vector3{0.0f, 0.0f, 0.0f};
    pointCloudBoundingRadius = 5.0f;

    // 重置加载状态
    isLoadingMesh      = false;
    currentLoadingFile = "";
    loadingProgress    = 0.0f;

    // 重置PCL处理状态
    isProcessingWithPCL   = false;
    currentProcessingFile = "";
    pclProcessingProgress = 0.0f;

    // 重置当前活跃文件
    currentActiveFile = "";

    WS_LOG_INFO("PointCloud",
                "Successfully cleared {} loaded meshes",
                clearedCount);
}

// 通过文件名切换点云Entity的函数（支持运行时加载）
bool World::switchToPointCloud(const std::string& filename,
                               ecs::Commands& commands)
{
    // 如果网格未加载，先加载
    if (loadedMeshes.find(filename) == loadedMeshes.end())
    {
        if (!loadPointCloudMesh(filename, commands))
        {
            WS_LOG_ERROR("PointCloud", "Failed to load mesh for {}", filename);
            return false;
        }
    }

    // 获取网格索引
    auto it = loadedMeshes.find(filename);
    if (it == loadedMeshes.end())
    {
        WS_LOG_ERROR("PointCloud",
                     "Mesh for {} not found after loading",
                     filename);
        return false;
    }

    // 如果已有点云Entity，先销毁
    if (pointCloudEntity != ecs::Entity::null())
    {
        commands.destroy(pointCloudEntity);
        pointCloudEntity = ecs::Entity::null();
        hasPointCloud    = false;
        WS_LOG_INFO("PointCloud", "Destroyed previous point cloud entity");
    }

    try
    {
        // 重新加载点云数据以获取包围盒信息（只用于计算，不生成网格）
        pointCloudData = pc::load(POINT_CLOUD_DIRECTORY + filename);

        // 更新点云中心和包围半径
        // 注意：这里的包围盒是变换前的原始数据，需要应用相同的变换
        math::Vector3 originalCenter = pointCloudData.volume.getCenter();
        math::Vector3 originalExtent = pointCloudData.volume.getExtent();
        float maxExtent              = originalExtent.elementMax();

        // 应用与PreProcess.hpp中相同的变换来计算变换后的中心
        // 1. 平移到原点：center - center = (0,0,0)
        // 2. 缩放：(0,0,0) * scale = (0,0,0)
        // 3. 旋转：应用X轴-90度旋转到(0,0,0) = (0,0,0)
        // 所以变换后的中心应该是原点
        pointCloudCenter = math::Vector3{0.0f, 0.0f, 0.0f};

        // 变换后的包围半径应该是标准化后的半径
        pointCloudBoundingRadius =
            0.5f; // 标准化后的点云最大范围应该是[-0.5, 0.5]

        // 获取必要的资源
        auto materials = commands.getResourceArray<StandardMaterial>();

        // 创建新的点云Entity，使用预加载的网格索引
        pointCloudEntity = commands.spawn(
            LocalTransform{.position = math::Vector3{0.0f, 0.0f, 0.0f},
                           .rotation = math::Quaternion::IDENTITY(),
                           .scale    = math::Vector3::ONE()},
            Mesh3D{it->second,
                   RHIPrimitiveTopology::PointList}, // 使用预加载的网格索引
            MeshMaterial{defaultPointMaterialIndex});

        hasPointCloud     = true;
        currentActiveFile = filename; // 设置当前活跃文件

        // 自动调整相机到最佳观察位置
        if (currentCamera != nullptr)
        {
            fitCameraToPointCloud(currentCamera);
        }

        WS_LOG_INFO("PointCloud",
                    "Switched to point cloud {}: {} points, entity: {}, "
                    "center: ({:.3f}, {:.3f}, {:.3f}), radius: {:.3f}",
                    filename,
                    pointCloudData.points.size(),
                    pointCloudEntity.toEntity(),
                    pointCloudCenter.x,
                    pointCloudCenter.y,
                    pointCloudCenter.z,
                    pointCloudBoundingRadius);

        return true;
    }
    catch (const std::exception& e)
    {
        WS_LOG_ERROR("PointCloud",
                     "Failed to switch to point cloud {}: {}",
                     filename,
                     e.what());
        return false;
    }
}