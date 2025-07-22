#include "View.hpp"
#include "../Application/World.hpp"
#include "../PointCloud/Process.hpp"

// 初始化可用文件列表的函数
void World::initializeLASFiles()
{
    availableFiles.clear();

    WS_LOG_INFO("PointCloud", "Scanning directory: {}", POINT_CLOUD_DIRECTORY);

    // 获取所有.las文件
    for (auto const& entry :
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

    WS_LOG_INFO("PointCloud", "Found {} point cloud files", availableFiles.size());
}

// 运行时加载点云网格的函数
bool World::loadCloudMesh(std::string const& filename,
                          ecs::Commands commands)
{
    // 检查是否已加载
    if (loadedMeshes.find(filename) != loadedMeshes.end())
    {
        WS_LOG_INFO("PointCloud", "{} already loaded", filename);
        return true;
    }

    isLoadingMesh      = true;
    currentLoadingFile = filename;

    std::string fullPath = POINT_CLOUD_DIRECTORY + filename;

    cloudData = pc::load(fullPath);

    if (!cloudData.points.empty())
    {
        // 创建网格并保存索引
        auto meshes           = commands.getResourceArray<Mesh>();
        std::size_t meshIndex = meshes.add(CustomMesh3D{
            .vertexType = RHIVertexType::Pos,
            .vertices   = {reinterpret_cast<std::byte*>(cloudData.points.data()),
                           cloudData.points.size() * sizeof(RHIVertexPos)},
        });

        meshes.get(meshIndex)->createGPUBuffers();

        loadedMeshes[filename] = meshIndex;

        WS_LOG_INFO("PointCloud", "Loaded mesh for {}: {} points, mesh index: {}", filename, cloudData.points.size(), meshIndex);

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

// 清理所有已加载的网格的函数
void World::clearAllLoadedMeshes(ecs::Commands commands)
{
    WS_LOG_INFO("PointCloud", "Clearing all loaded meshes...");

    // 如果有当前活跃的点云Entity，先销毁
    if (cloudEntity != ecs::Entity::null())
    {
        commands.destroy(cloudEntity);
        cloudEntity = ecs::Entity::null();
        hasCloud    = false;
        WS_LOG_INFO("PointCloud", "Destroyed current point cloud entity");
    }

    // 获取网格资源数组
    auto meshes = commands.getResourceArray<Mesh>();

    // 遍历所有已加载的网格并释放
    for (const auto& [filename, meshIndex] : loadedMeshes)
    {

        // 尝试获取并清理网格
        if (auto* mesh = meshes.get(meshIndex))
        {
            mesh->clear();
            WS_LOG_INFO("PointCloud", "Cleared mesh for {}: index {}", filename, meshIndex);
        }
    }

    // 清空映射表
    size_t clearedCount = loadedMeshes.size();
    loadedMeshes.clear();

    // 重置点云相关状态
    cloudData = pc::Cloud{}; // 重置为空

    // 重置加载状态
    isLoadingMesh      = false;
    currentLoadingFile = "";

    // 重置PCL处理状态
    isProcessing          = false;
    currentProcessingFile = "";

    // 重置当前活跃文件
    currentActiveFile = "";

    WS_LOG_INFO("PointCloud", "Successfully cleared {} loaded meshes", clearedCount);
}

// 通过文件名切换点云Entity的函数（支持运行时加载）
bool World::switchToPointCloud(std::string const& filename,
                               ecs::Commands commands)
{
    // 如果网格未加载，先加载
    if (loadedMeshes.find(filename) == loadedMeshes.end())
    {
        if (!loadCloudMesh(filename, commands))
        {
            WS_LOG_ERROR("PointCloud", "Failed to load mesh for {}", filename);
            return false;
        }
    }

    // 获取网格索引
    auto it = loadedMeshes.find(filename);
    if (it == loadedMeshes.end())
    {
        WS_LOG_ERROR("PointCloud", "Mesh for {} not found after loading", filename);
        return false;
    }

    // 如果已有点云Entity，先销毁
    if (cloudEntity != ecs::Entity::null())
    {
        commands.destroy(cloudEntity);
        cloudEntity = ecs::Entity::null();
        hasCloud    = false;
        WS_LOG_INFO("PointCloud", "Destroyed previous point cloud entity");
    }

    // 获取必要的资源
    auto materials = commands.getResourceArray<StandardMaterial>();

    // clang-format off
    static constexpr math::Matrix3 tranform = {
        1.0f,  0.0f, 0.0f,
        0.0f,  0.0f, 1.0f,
        0.0f, -1.0f, 0.0f,
    };
    // clang-format on

    // 创建新的点云Entity，使用预加载的网格索引
    cloudEntity = commands.spawn(
        LocalTransform{
            .rotation = math::Quaternion::fromMat3(tranform)},
        Mesh3D{it->second,
               RHIPrimitiveTopology::PointList}, // 使用预加载的网格索引
        MeshMaterial{defaultMaterial});

    hasCloud          = true;
    currentActiveFile = filename; // 设置当前活跃文件

    float const scale      = cloudData.volume.getExtent().elementMax();
    Camera* camera         = commands.getResource<Camera>().get();
    CameraData* cameraData = commands.getResource<CameraData>().get();
    fitView(camera, cameraData, math::Vector3::ZERO(), scale);

    return true;
}

bool World::processMesh(std::string const& filename, ecs::Commands commands)
{
    auto it = loadedMeshes.find(filename);
    if (it == loadedMeshes.end())
    {
        WS_LOG_ERROR("Open3D", "Process failed. {} not loaded", filename);
        return false;
    }

    isProcessing          = true;
    currentProcessingFile = filename;

    // 1.加载点云数据
    std::string fullPath = POINT_CLOUD_DIRECTORY + filename;

    // 使用Open3D加载点云
    std::shared_ptr<open3d::geometry::PointCloud> o3dCloud = std::make_shared<open3d::geometry::PointCloud>();
    pc::Cloud originalCloud                                = pc::load(fullPath);

    if (originalCloud.points.empty())
    {
        WS_LOG_ERROR("Open3D", "No points in {}", filename);
        isProcessing          = false;
        currentProcessingFile = "";
        return false;
    }

    // 2. 转换为 Open3D 点云格式
    o3dCloud->points_.reserve(originalCloud.points.size());
    for (RHIVertexPos const& point : originalCloud.points)
    {
        o3dCloud->points_.emplace_back(point.position.x, point.position.y, point.position.z);
    }

    // 3.1 SOR 降噪
    auto [cleanCloud, indices] = o3dCloud->RemoveStatisticalOutliers(20, 2.0);
    WS_LOG_INFO("Open3D", "SOR : {} -> {} points", o3dCloud->points_.size(), cleanCloud->points_.size());

    // 3.2 地面分割
    auto groundResult = segmentGround(cleanCloud, 0.2, 3, 1000);

    WS_LOG_INFO("Open3D",
                "Ground segmentation: {} ground points, {} non-ground points",
                groundResult.groundPoints->points_.size(),
                groundResult.nonGroundPoints->points_.size());

    // 3.3 非地面点聚类分析
    auto clusteringResult =
        performClustering(groundResult.nonGroundPoints, 1.0, 50);

    WS_LOG_INFO("Open3D", "Found {} clusters", clusteringResult.clusters.size());

    // 3.4 电力基础设施分类
    WS_LOG_INFO("Open3D", "Classifying power infrastructure...");
    auto infra = classifyInfrastructure(clusteringResult.clusters,
                                        groundResult.groundPoints);

    // 3.5 电力线曲线拟合
    WS_LOG_INFO("Open3D", "Fitting power line curves...");
    for (const auto& power_line : infra.powerLines)
    {
        auto curve = fitPowerLineCurve(power_line);
        infra.powerLineCurves.push_back(curve);
    }

    // 记录分析结果
    if (infra.towerPoints)
    {
        WS_LOG_INFO("Open3D", "Identified power tower with {} points", infra.towerPoints->points_.size());
        WS_LOG_INFO("Open3D",
                    "Tower bounding box: ({:.2f}, {:.2f}, {:.2f}) to "
                    "({:.2f}, {:.2f}, {:.2f})",
                    infra.towerBbox.getMin().x,
                    infra.towerBbox.getMin().y,
                    infra.towerBbox.getMin().z,
                    infra.towerBbox.getMax().x,
                    infra.towerBbox.getMax().y,
                    infra.towerBbox.getMax().z);
    }

    WS_LOG_INFO("Open3D", "Identified {} power lines", infra.powerLines.size());
    for (size_t i = 0; i < infra.powerLines.size(); ++i)
    {
        WS_LOG_INFO("Open3D", "Power line {}: {} points, {} curve control points", i + 1, infra.powerLines[i]->points_.size(), infra.powerLineCurves[i].size());
    }

    // 生成电力线参数并触发弹出窗口
    // generatePowerLineParameters(infra, filename, commands);

    WS_LOG_INFO("Open3D", "{} processing completed ", filename);

    isProcessing          = false;
    currentProcessingFile = "";
    return true;
}