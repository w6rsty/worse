#include "CloudStorage.hpp"
#include "View.hpp"
#include "../Application/World.hpp"
#include "../PointCloud/Process.hpp"

#include <open3d/geometry/BoundingVolume.h>
#include <open3d/geometry/PointCloud.h>
#include <open3d/geometry/KDTreeFlann.h>
#include <open3d/pipelines/registration/Registration.h>

#include <set>
#include <memory>

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
bool World::assureCloudMesh(std::string const& filepath,
                            ecs::Commands commands)
{
    // 检查是否已加载
    if (cloudStorageManager.has(filepath))
    {
        WS_LOG_INFO("PointCloud", "{} already loaded", filepath);
        return true;
    }

    CloudStorage& cloudStorage = cloudStorageManager.load(filepath);
    cloudStorage.createSubCloud("main", [](pc::Cloud const& cloud)
                                {
                                    return std::vector<RHIVertexPos>(cloud.points.begin(), cloud.points.end());
                                });

    WS_LOG_INFO("PointCloud", "Loaded {}", filepath);

    return true;
}

// 清理所有已加载的网格的函数
void World::clearAllLoadedMeshes(ecs::Commands commands)
{
    std::size_t clearedCount = cloudStorageManager.size();

    cloudStorageManager.destroyEntities(commands);
    cloudStorageManager.clear();
    hasCloud = false;

    // 重置当前活跃文件
    currentActiveFile = "";

    WS_LOG_INFO("PointCloud", "Cleared {} clouds", clearedCount);
}

// 通过文件名切换点云Entity的函数（支持运行时加载）
bool World::switchToPointCloud(std::string const& filename,
                               ecs::Commands commands)
{
    CloudStorage* lastStorage = cloudStorageManager.get(POINT_CLOUD_DIRECTORY + currentActiveFile);

    // 销毁先前的点云
    if (lastStorage)
    {
        auto lastSubCloudKeys = lastStorage->getSubCloudKeys();
        int destroyedCount    = 0;
        for (const auto& key : lastSubCloudKeys)
        {
            const auto& subCloud = lastStorage->getSubCloud(key);
            if (subCloud.entity != ecs::Entity::null())
            {
                destroyedCount++;
            }
        }

        lastStorage->destroyEntities(commands);
        hasCloud = false;
    }

    std::string fullPath = POINT_CLOUD_DIRECTORY + filename;
    if (!assureCloudMesh(fullPath, commands))
    {
        WS_LOG_ERROR("PointCloud", "Failed to load {}", filename);
        return false;
    }
    CloudStorage* storage = cloudStorageManager.get(fullPath);

    // 提交点云绘制
    {
        // clang-format off
        static constexpr math::Matrix3 tranform = {
            1.0f,  0.0f, 0.0f,
            0.0f,  0.0f, 1.0f,
            0.0f, -1.0f, 0.0f,
        };
        // clang-format on

        // 创建新的点云Entity，使用预加载的网格索引
        SubCloud& mainSubCloud = cloudStorageManager.get(fullPath)->getSubCloud("main");
        mainSubCloud.entity    = commands.spawn(
            LocalTransform{
                   .rotation = math::Quaternion::fromMat3(tranform)},
            Mesh3D{mainSubCloud.mesh,
                   RHIPrimitiveTopology::PointList}, // 使用预加载的网格索引
            MeshMaterial{defaultMaterial},
            CloudTag{});
    }

    hasCloud          = true;
    currentActiveFile = filename; // 设置当前活跃文件

    float const scale      = cloudStorageManager.get(POINT_CLOUD_DIRECTORY + currentActiveFile)->getMasterCloud().volume.getExtent().elementMax();
    Camera* camera         = commands.getResource<Camera>().get();
    CameraData* cameraData = commands.getResource<CameraData>().get();
    fitView(camera, cameraData, math::Vector3::ZERO(), scale);

    return true;
}

bool World::processMesh(std::string const& filename, ecs::Commands commands)
{
    std::string fullPath = POINT_CLOUD_DIRECTORY + filename;
    if (!cloudStorageManager.has(fullPath))
    {
        WS_LOG_ERROR("Open3D", "Process failed. {} not loaded", filename);
        return false;
    }

    WS_LOG_INFO("Open3D", "Starting advanced point cloud processing for {}", filename);

    // 1. 加载和预处理点云数据
    std::shared_ptr<open3d::geometry::PointCloud> o3dCloud = std::make_shared<open3d::geometry::PointCloud>();
    pc::Cloud const& originalCloud                         = cloudStorageManager.get(fullPath)->getMasterCloud();

    if (originalCloud.points.empty())
    {
        WS_LOG_ERROR("Open3D", "No points in {}", filename);
        return false;
    }

    // 2. 高效转换为 Open3D 点云格式
    o3dCloud->points_.reserve(originalCloud.points.size());
    for (RHIVertexPos const& point : originalCloud.points)
    {
        o3dCloud->points_.emplace_back(point.position.x, point.position.y, point.position.z);
    }

    WS_LOG_INFO("Open3D", "Loaded {} points for processing", o3dCloud->points_.size());

    // 3. 高级多阶段点云预处理

    // 3.1 质量评估
    auto qualityMetrics = assessPointCloudQuality(o3dCloud);
    WS_LOG_INFO("Open3D", "Point cloud quality assessment completed");

    // 3.2 基于质量的自适应预处理
    auto bbox                  = o3dCloud->GetAxisAlignedBoundingBox();
    auto extent                = bbox.GetExtent();
    double adaptive_voxel_size = std::min({extent.x(), extent.y(), extent.z()}) / 150.0;
    adaptive_voxel_size        = std::max(adaptive_voxel_size, 0.05); // 最小体素大小

    // 根据噪点比例调整预处理强度
    int sor_neighbors    = qualityMetrics.noise_ratio > 0.1 ? 30 : 20;
    double sor_std_ratio = qualityMetrics.noise_ratio > 0.1 ? 1.2 : 1.5;

    auto preprocessedCloud = preprocessPointCloud(o3dCloud, adaptive_voxel_size, sor_neighbors, sor_std_ratio, 16, 0.4);

    WS_LOG_INFO("Open3D", "Advanced preprocessing completed: {} -> {} points", o3dCloud->points_.size(), preprocessedCloud->points_.size());

    // 4. 改进的地面分割算法

    // 安全检查：确保有足够的点进行分割
    if (preprocessedCloud->points_.size() < 1000)
    {
        WS_LOG_ERROR("Open3D", "Insufficient points ({}) for ground segmentation", preprocessedCloud->points_.size());
        return false;
    }

    // 4.1 使用RANSAC进行初始平面检测
    std::shared_ptr<open3d::geometry::PointCloud> ground_ransac;
    std::shared_ptr<open3d::geometry::PointCloud> non_ground_ransac;

    try
    {
        auto [plane_model, inliers] = preprocessedCloud->SegmentPlane(0.3, 3, 2000);
        ground_ransac               = preprocessedCloud->SelectByIndex(inliers);
        non_ground_ransac           = preprocessedCloud->SelectByIndex(inliers, true);

        WS_LOG_INFO("Open3D", "RANSAC plane segmentation: {} ground, {} non-ground points", ground_ransac->points_.size(), non_ground_ransac->points_.size());
    }
    catch (const std::exception& e)
    {
        WS_LOG_WARN("Open3D", "RANSAC segmentation failed: {}, using fallback method", e.what());

        // 回退方法：基于高度的简单分割
        auto bbox               = preprocessedCloud->GetAxisAlignedBoundingBox();
        double ground_threshold = bbox.GetMinBound().z() + (bbox.GetExtent().z() * 0.1);

        std::vector<size_t> ground_indices, non_ground_indices;
        for (size_t i = 0; i < preprocessedCloud->points_.size(); ++i)
        {
            if (preprocessedCloud->points_[i].z() <= ground_threshold)
            {
                ground_indices.push_back(i);
            }
            else
            {
                non_ground_indices.push_back(i);
            }
        }

        ground_ransac     = preprocessedCloud->SelectByIndex(ground_indices);
        non_ground_ransac = preprocessedCloud->SelectByIndex(non_ground_indices);

        WS_LOG_INFO("Open3D", "Height-based fallback segmentation: {} ground, {} non-ground points", ground_ransac->points_.size(), non_ground_ransac->points_.size());
    }

    // 4.2 结合渐进形态学滤波进行精细分割（如果RANSAC成功）
    GroundSegmentationResult groundResult;
    if (ground_ransac && non_ground_ransac && !ground_ransac->points_.empty())
    {
        try
        {
            groundResult = segmentGround(preprocessedCloud, 0.15, 3, 1000);
        }
        catch (const std::exception& e)
        {
            WS_LOG_WARN("Open3D", "Morphological segmentation failed: {}, using RANSAC results", e.what());
        }
    }

    // 4.3 选择最佳分割结果
    std::shared_ptr<open3d::geometry::PointCloud> finalGroundCloud;
    std::shared_ptr<open3d::geometry::PointCloud> finalNonGroundCloud;

    if (groundResult.groundPoints && groundResult.nonGroundPoints &&
        !groundResult.groundPoints->points_.empty() && !groundResult.nonGroundPoints->points_.empty())
    {
        // 使用形态学滤波结果
        finalGroundCloud    = groundResult.groundPoints;
        finalNonGroundCloud = groundResult.nonGroundPoints;

        WS_LOG_INFO("Open3D", "Using morphological segmentation: {} ground, {} non-ground points", finalGroundCloud->points_.size(), finalNonGroundCloud->points_.size());
    }
    else if (ground_ransac && non_ground_ransac)
    {
        // 回退到RANSAC结果
        finalGroundCloud    = ground_ransac;
        finalNonGroundCloud = non_ground_ransac;
        WS_LOG_INFO("Open3D", "Using RANSAC segmentation results");
    }
    else
    {
        WS_LOG_ERROR("Open3D", "All ground segmentation methods failed");
        return false;
    }

    // 5. 生成改进的非地面网格
    {
        std::vector<RHIVertexPos> nonGroundPoints;
        nonGroundPoints.reserve(finalNonGroundCloud->points_.size());
        for (auto const& point : finalNonGroundCloud->points_)
        {
            nonGroundPoints.push_back(
                RHIVertexPos{{static_cast<float>(point.x()),
                              static_cast<float>(point.y()),
                              static_cast<float>(point.z())}});
        }
        cloudStorageManager.get(fullPath)->createSubCloud("nonGround",
                                                          [nonGroundPoints](pc::Cloud const&) -> std::vector<RHIVertexPos>
                                                          {
                                                              return nonGroundPoints;
                                                          });

        // clang-format off
        static constexpr math::Matrix3 tranform = {
            1.0f,  0.0f, 0.0f,
            0.0f,  0.0f, 1.0f,
            0.0f, -1.0f, 0.0f,
        };
        // clang-format on

        // 创建Entity并记录到SubCloud中
        SubCloud& nonGroundSubCloud = cloudStorageManager.get(fullPath)->getSubCloud("nonGround");
        nonGroundSubCloud.entity    = commands.spawn(
            LocalTransform{
                   .rotation = math::Quaternion::fromMat3(tranform)},
            Mesh3D{nonGroundSubCloud.mesh,
                   RHIPrimitiveTopology::PointList},
            MeshMaterial{1},
            CloudTag{});
    }

    // 6. 智能自适应聚类分析

    ClusteringResult clusteringResult;

    // 安全检查：确保有足够的非地面点进行聚类
    if (!finalNonGroundCloud || finalNonGroundCloud->points_.size() < 100)
    {
        WS_LOG_WARN("Open3D", "Insufficient non-ground points ({}) for clustering", finalNonGroundCloud ? finalNonGroundCloud->points_.size() : 0);
    }
    else
    {
        // 6.1 使用新的自适应参数估计函数
        auto clusteringParams = estimateClusteringParameters(finalNonGroundCloud, 1.0, 50);

        WS_LOG_INFO("Open3D", "Intelligent clustering parameters - eps: {:.2f}, min_points: {}, density: {:.2f}", clusteringParams.eps, clusteringParams.min_points, clusteringParams.density_threshold);

        // 6.2 执行优化的聚类算法
        try
        {
            clusteringResult = performClustering(finalNonGroundCloud,
                                                 clusteringParams.eps,
                                                 clusteringParams.min_points);

            WS_LOG_INFO("Open3D", "Intelligent clustering completed: {} high-quality clusters", clusteringResult.clusters.size());
        }
        catch (const std::exception& e)
        {
            WS_LOG_WARN("Open3D", "Clustering failed: {}, continuing without clusters", e.what());
        }
    }

    // 7. 智能电力基础设施分类
    WS_LOG_INFO("Open3D", "Performing intelligent power infrastructure classification...");
    auto infra = classifyInfrastructure(clusteringResult.clusters, finalGroundCloud);

    // 8. 创建高质量地面点云子网格
    if (finalGroundCloud && !finalGroundCloud->points_.empty())
    {
        std::vector<RHIVertexPos> groundPoints;
        groundPoints.reserve(finalGroundCloud->points_.size());
        for (auto const& point : finalGroundCloud->points_)
        {
            groundPoints.push_back(
                RHIVertexPos{{static_cast<float>(point.x()),
                              static_cast<float>(point.y()),
                              static_cast<float>(point.z())}});
        }

        cloudStorageManager.get(fullPath)->createSubCloud("ground",
                                                          [groundPoints](pc::Cloud const&) -> std::vector<RHIVertexPos>
                                                          {
                                                              return groundPoints;
                                                          });

        // clang-format off
        static constexpr math::Matrix3 tranform = {
            1.0f,  0.0f, 0.0f,
            0.0f,  0.0f, 1.0f,
            0.0f, -1.0f, 0.0f,
        };
        // clang-format on

        // 创建Entity并记录到SubCloud中
        SubCloud& groundSubCloud = cloudStorageManager.get(fullPath)->getSubCloud("ground");
        groundSubCloud.entity    = commands.spawn(
            LocalTransform{
                   .rotation = math::Quaternion::fromMat3(tranform)},
            Mesh3D{groundSubCloud.mesh,
                   RHIPrimitiveTopology::PointList},
            MeshMaterial{1},
            CloudTag{});

        WS_LOG_INFO("Open3D", "Created enhanced ground subcloud with {} points", groundPoints.size());
    }

    // 创建电力塔子网格
    if (infra.towerPoints && !infra.towerPoints->points_.empty())
    {
        std::vector<RHIVertexPos> towerPoints;
        towerPoints.reserve(infra.towerPoints->points_.size());
        for (auto const& point : infra.towerPoints->points_)
        {
            towerPoints.push_back(
                RHIVertexPos{{static_cast<float>(point.x()),
                              static_cast<float>(point.y()),
                              static_cast<float>(point.z())}});
        }

        cloudStorageManager.get(fullPath)->createSubCloud("tower",
                                                          [towerPoints](pc::Cloud const&) -> std::vector<RHIVertexPos>
                                                          {
                                                              return towerPoints;
                                                          });

        // 创建电力塔渲染实体
        // clang-format off
        static constexpr math::Matrix3 tranform = {
            1.0f,  0.0f, 0.0f,
            0.0f,  0.0f, 1.0f,
            0.0f, -1.0f, 0.0f,
        };
        // clang-format on

        // 创建Entity并记录到SubCloud中
        SubCloud& towerSubCloud = cloudStorageManager.get(fullPath)->getSubCloud("tower");
        towerSubCloud.entity    = commands.spawn(
            LocalTransform{
                   .rotation = math::Quaternion::fromMat3(tranform)},
            Mesh3D{towerSubCloud.mesh,
                   RHIPrimitiveTopology::PointList},
            MeshMaterial{2}, // 不同的材质ID用于区分
            CloudTag{});

        WS_LOG_INFO("Open3D", "Created tower subcloud with {} points", towerPoints.size());
    }

    // 创建电力线子网格
    for (size_t i = 0; i < infra.powerLines.size(); ++i)
    {
        const auto& powerLine = infra.powerLines[i];
        if (powerLine && !powerLine->points_.empty())
        {
            std::vector<RHIVertexPos> powerLinePoints;
            powerLinePoints.reserve(powerLine->points_.size());
            for (auto const& point : powerLine->points_)
            {
                powerLinePoints.push_back(
                    RHIVertexPos{{static_cast<float>(point.x()),
                                  static_cast<float>(point.y()),
                                  static_cast<float>(point.z())}});
            }

            std::string subCloudKey = "powerLine_" + std::to_string(i + 1);
            cloudStorageManager.get(fullPath)->createSubCloud(subCloudKey,
                                                              [powerLinePoints](pc::Cloud const&) -> std::vector<RHIVertexPos>
                                                              {
                                                                  return powerLinePoints;
                                                              });

            // 创建电力线渲染实体
            // clang-format off
            static constexpr math::Matrix3 tranform = {
                1.0f,  0.0f, 0.0f,
                0.0f,  0.0f, 1.0f,
                0.0f, -1.0f, 0.0f,
            };
            // clang-format on

            // 创建Entity并记录到SubCloud中
            SubCloud& powerLineSubCloud = cloudStorageManager.get(fullPath)->getSubCloud(subCloudKey);
            powerLineSubCloud.entity    = commands.spawn(
                LocalTransform{
                       .rotation = math::Quaternion::fromMat3(tranform)},
                Mesh3D{powerLineSubCloud.mesh,
                       RHIPrimitiveTopology::PointList},
                MeshMaterial{static_cast<std::size_t>(3 + (i % 5))}, // 使用不同的材质ID，循环使用5个材质
                CloudTag{});

            WS_LOG_INFO("Open3D", "Created power line {} subcloud with {} points", i + 1, powerLinePoints.size());
        }
    }

    // 创建其他聚类的子网格（除了已经分类为电力塔和电力线的）
    std::set<std::shared_ptr<open3d::geometry::PointCloud>> classifiedClusters;
    if (infra.towerPoints)
    {
        classifiedClusters.insert(infra.towerPoints);
    }
    for (const auto& powerLine : infra.powerLines)
    {
        classifiedClusters.insert(powerLine);
    }

    int otherClusterIndex = 1;
    for (const auto& cluster : clusteringResult.clusters)
    {
        // 跳过已经分类的聚类
        if (classifiedClusters.find(cluster) != classifiedClusters.end())
        {
            continue;
        }

        if (cluster && !cluster->points_.empty())
        {
            std::vector<RHIVertexPos> clusterPoints;
            clusterPoints.reserve(cluster->points_.size());
            for (auto const& point : cluster->points_)
            {
                clusterPoints.push_back(
                    RHIVertexPos{{static_cast<float>(point.x()),
                                  static_cast<float>(point.y()),
                                  static_cast<float>(point.z())}});
            }

            std::string subCloudKey = "cluster_" + std::to_string(otherClusterIndex);
            cloudStorageManager.get(fullPath)->createSubCloud(subCloudKey,
                                                              [clusterPoints](pc::Cloud const&) -> std::vector<RHIVertexPos>
                                                              {
                                                                  return clusterPoints;
                                                              });

            // 创建其他聚类渲染实体
            // clang-format off
            static constexpr math::Matrix3 tranform = {
                1.0f,  0.0f, 0.0f,
                0.0f,  0.0f, 1.0f,
                0.0f, -1.0f, 0.0f,
            };
            // clang-format on

            // 创建Entity并记录到SubCloud中
            SubCloud& clusterSubCloud = cloudStorageManager.get(fullPath)->getSubCloud(subCloudKey);
            clusterSubCloud.entity    = commands.spawn(
                LocalTransform{
                       .rotation = math::Quaternion::fromMat3(tranform)},
                Mesh3D{clusterSubCloud.mesh,
                       RHIPrimitiveTopology::PointList},
                MeshMaterial{static_cast<std::size_t>(8 + (otherClusterIndex % 4))}, // 使用不同的材质ID
                CloudTag{});

            WS_LOG_INFO("Open3D", "Created cluster {} subcloud with {} points", otherClusterIndex, clusterPoints.size());
            otherClusterIndex++;
        }
    }

    // 9. 高精度电力线曲线拟合和分析
    WS_LOG_INFO("Open3D", "Performing advanced power line curve fitting...");

    // 9.1 为每条电力线生成高精度曲线模型
    for (const auto& power_line : infra.powerLines)
    {
        if (power_line && !power_line->points_.empty())
        {
            // 执行曲线拟合
            auto curve = fitPowerLineCurve(power_line);
            infra.powerLineCurves.push_back(curve);

            // 计算电力线的物理特性
            auto bbox     = power_line->GetAxisAlignedBoundingBox();
            auto extent   = bbox.GetExtent();
            double length = extent.norm(); // 使用Eigen的norm()函数计算长度

            // 计算简单的垂度估算（高度差的一半）
            double height_diff   = extent.z();
            double estimated_sag = height_diff * 0.1; // 简化的垂度估算

            WS_LOG_INFO("Open3D", "Power line analysis - Length: {:.2f}m, Estimated sag: {:.2f}m", length, estimated_sag);
        }
    }

    // 9.2 高级分析结果汇总
    if (infra.towerPoints)
    {
        // 计算电力塔的详细几何特征
        auto towerBbox     = infra.towerPoints->GetAxisAlignedBoundingBox();
        auto towerExtent   = towerBbox.GetExtent();
        double towerHeight = towerExtent.z();
        double towerBase   = std::max(towerExtent.x(), towerExtent.y());

        WS_LOG_INFO("Open3D", "Power tower analysis - {} points, Height: {:.2f}m, Base: {:.2f}m", infra.towerPoints->points_.size(), towerHeight, towerBase);
        WS_LOG_INFO("Open3D",
                    "Tower bounding box: ({:.2f}, {:.2f}, {:.2f}) to ({:.2f}, {:.2f}, {:.2f})",
                    infra.towerMin.x,
                    infra.towerMin.y,
                    infra.towerMin.z,
                    infra.towerMax.x,
                    infra.towerMax.y,
                    infra.towerMax.z);
    }

    WS_LOG_INFO("Open3D", "Infrastructure summary - {} power lines, {} curve control points total", infra.powerLines.size(), std::accumulate(infra.powerLineCurves.begin(), infra.powerLineCurves.end(), 0, [](int sum, const auto& curve)
                                                                                                                                             {
                                                                                                                                                 return sum + curve.size();
                                                                                                                                             }));

    for (size_t i = 0; i < infra.powerLines.size(); ++i)
    {
        if (i < infra.powerLineCurves.size())
        {
            WS_LOG_INFO("Open3D", "Power line {}: {} points, {} curve control points", i + 1, infra.powerLines[i]->points_.size(), infra.powerLineCurves[i].size());
        }
    }

    // 10. 内存优化和性能提升

    // 10.1 清理中间处理数据以释放内存
    o3dCloud.reset();
    preprocessedCloud.reset();
    ground_ransac.reset();
    non_ground_ransac.reset();

    WS_LOG_INFO("Open3D", "Memory optimization: Cleared intermediate processing data");

    // 10.2 生成处理报告
    WS_LOG_INFO("Open3D", "=== Point Cloud Processing Summary ===");
    WS_LOG_INFO("Open3D", "Original points: {}", originalCloud.points.size());
    WS_LOG_INFO("Open3D", "Ground points: {}", finalGroundCloud ? finalGroundCloud->points_.size() : 0);
    WS_LOG_INFO("Open3D", "Non-ground points: {}", finalNonGroundCloud ? finalNonGroundCloud->points_.size() : 0);
    WS_LOG_INFO("Open3D", "Detected clusters: {}", clusteringResult.clusters.size());
    WS_LOG_INFO("Open3D", "Power infrastructure: {} towers, {} power lines", infra.towerPoints ? 1 : 0, infra.powerLines.size());

    // 统计创建的SubCloud和Entity数量
    auto* storage = cloudStorageManager.get(fullPath);
    if (storage)
    {
        auto subCloudKeys = storage->getSubCloudKeys();
        int entityCount   = 0;
        for (const auto& key : subCloudKeys)
        {
            if (key != "main") // 排除主点云，只计算处理生成的
            {
                const auto& subCloud = storage->getSubCloud(key);
                if (subCloud.entity != ecs::Entity::null())
                {
                    entityCount++;
                }
            }
        }
        WS_LOG_INFO("Open3D", "Created {} SubClouds with {} active entities", subCloudKeys.size() - 1, entityCount); // -1 排除main

        // 11. 自动隐藏大型子点云以突出显示分析结果
        storage->autoHideLargeSubClouds(5.0); // 隐藏点数超过中位数3倍的子点云
        WS_LOG_INFO("Open3D", "Applied auto-hide for large SubClouds to highlight analysis results");
    }

    // 生成电力线参数并触发弹出窗口（启用改进的分析）
    PowerLineMergeStats defaultStats;
    defaultStats.originalSegmentCount = infra.powerLines.size();
    defaultStats.mergedLineCount      = infra.powerLines.size();
    defaultStats.averageMergeQuality  = 1.0;
    defaultStats.processingNote       = "智能聚类处理完成";
    for (size_t i = 0; i < infra.powerLines.size(); ++i)
    {
        defaultStats.segmentCounts.push_back(1);
    }

    generatePowerLineParameters(infra, filename, commands, defaultStats);

    WS_LOG_INFO("Open3D", "Advanced point cloud processing completed successfully for {}", filename);

    return true;
}