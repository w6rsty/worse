#include <open3d/geometry/KDTreeFlann.h>
#include <open3d/geometry/BoundingVolume.h>
#include <open3d/io/PointCloudIO.h>

#include "Process.hpp"
#include "../Application/World.hpp"

using namespace worse;

// 渐进形态滤波地面分割
GroundSegmentationResult
segmentGround(std::shared_ptr<open3d::geometry::PointCloud> cloud,
              double cellSize, double maxWindowSize, double slopeThreshold,
              double maxDistance, double initialDistance)
{
    GroundSegmentationResult result;

    if (cloud->points_.empty())
    {
        return result;
    }

    // 1. 计算点云边界
    auto bbox     = cloud->GetAxisAlignedBoundingBox();
    auto minBound = bbox.GetMinBound();
    auto maxBound = bbox.GetMaxBound();

    // 2. 创建网格
    int gridWidth =
        static_cast<int>((maxBound.x() - minBound.x()) / cellSize) + 1;
    int gridHeight =
        static_cast<int>((maxBound.y() - minBound.y()) / cellSize) + 1;

    // 3. 初始化高度网格（存储每个网格单元的最小高度）
    std::vector<std::vector<double>> heightGrid(
        gridWidth,
        std::vector<double>(gridHeight, std::numeric_limits<double>::max()));
    std::vector<std::vector<std::vector<size_t>>> pointGrid(
        gridWidth,
        std::vector<std::vector<size_t>>(gridHeight));

    // 4. 将点投影到网格并记录最小高度
    for (std::size_t i = 0; i < cloud->points_.size(); ++i)
    {
        const auto& point = cloud->points_[i];
        int x             = static_cast<int>((point.x() - minBound.x()) / cellSize);
        int y             = static_cast<int>((point.y() - minBound.y()) / cellSize);

        if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight)
        {
            heightGrid[x][y] = std::min(heightGrid[x][y], point.z());
            pointGrid[x][y].push_back(i);
        }
    }

    // 5. 渐进形态滤波
    std::vector<std::vector<double>> filteredHeight = heightGrid;

    // 多尺度形态学开运算
    for (double windowSize = 1.0; windowSize <= maxWindowSize; windowSize *= 2)
    {
        int halfWindow                            = static_cast<int>(windowSize / cellSize / 2);
        std::vector<std::vector<double>> tempGrid = filteredHeight;

        // 形态学开运算 = 腐蚀 + 膨胀
        // 腐蚀操作
        for (int x = 0; x < gridWidth; ++x)
        {
            for (int y = 0; y < gridHeight; ++y)
            {
                if (filteredHeight[x][y] == std::numeric_limits<double>::max())
                    continue;

                double minHeight = filteredHeight[x][y];
                for (int dx = -halfWindow; dx <= halfWindow; ++dx)
                {
                    for (int dy = -halfWindow; dy <= halfWindow; ++dy)
                    {
                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && nx < gridWidth && ny >= 0 &&
                            ny < gridHeight &&
                            filteredHeight[nx][ny] !=
                                std::numeric_limits<double>::max())
                        {
                            minHeight =
                                std::min(minHeight, filteredHeight[nx][ny]);
                        }
                    }
                }
                tempGrid[x][y] = minHeight;
            }
        }

        // 膨胀操作
        filteredHeight = tempGrid;
        for (int x = 0; x < gridWidth; ++x)
        {
            for (int y = 0; y < gridHeight; ++y)
            {
                if (tempGrid[x][y] == std::numeric_limits<double>::max())
                    continue;

                double maxHeight = tempGrid[x][y];
                for (int dx = -halfWindow; dx <= halfWindow; ++dx)
                {
                    for (int dy = -halfWindow; dy <= halfWindow; ++dy)
                    {
                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && nx < gridWidth && ny >= 0 &&
                            ny < gridHeight &&
                            tempGrid[nx][ny] !=
                                std::numeric_limits<double>::max())
                        {
                            maxHeight = std::max(maxHeight, tempGrid[nx][ny]);
                        }
                    }
                }
                filteredHeight[x][y] = maxHeight;
            }
        }
    }

    // 6. 根据高度差和坡度分类点
    std::vector<size_t> groundIndices, nonGroundIndices;

    for (size_t i = 0; i < cloud->points_.size(); ++i)
    {
        const auto& point = cloud->points_[i];
        int x             = static_cast<int>((point.x() - minBound.x()) / cellSize);
        int y             = static_cast<int>((point.y() - minBound.y()) / cellSize);

        if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight &&
            filteredHeight[x][y] != std::numeric_limits<double>::max())
        {
            double heightDiff = point.z() - filteredHeight[x][y];

            // 计算局部坡度
            double slope   = 0.0;
            int slopeCount = 0;
            for (int dx = -1; dx <= 1; ++dx)
            {
                for (int dy = -1; dy <= 1; ++dy)
                {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < gridWidth && ny >= 0 &&
                        ny < gridHeight &&
                        filteredHeight[nx][ny] !=
                            std::numeric_limits<double>::max())
                    {
                        double distance =
                            std::sqrt(dx * dx + dy * dy) * cellSize;
                        if (distance > 0)
                        {
                            slope += std::abs(filteredHeight[nx][ny] -
                                              filteredHeight[x][y]) /
                                     distance;
                            ++slopeCount;
                        }
                    }
                }
            }
            if (slopeCount > 0)
            {
                slope /= slopeCount;
            }

            // 动态阈值：坡度越大，允许的高度差越大
            double adaptiveThreshold = initialDistance + slope * maxDistance;

            if (heightDiff <= adaptiveThreshold && slope <= slopeThreshold)
            {
                groundIndices.push_back(i);
            }
            else
            {
                nonGroundIndices.push_back(i);
            }
        }
        else
        {
            nonGroundIndices.push_back(i);
        }
    }

    // 7. 创建结果点云
    result.groundPoints    = cloud->SelectByIndex(groundIndices);
    result.nonGroundPoints = cloud->SelectByIndex(nonGroundIndices);

    // 构造地面平面模型（简化为水平面）
    if (!groundIndices.empty())
    {
        double avgZ = 0.0;
        for (size_t idx : groundIndices)
        {
            avgZ += cloud->points_[idx].z();
        }
        avgZ /= groundIndices.size();
        result.groundPlaneModel = Eigen::Vector4d(0, 0, 1, -avgZ);
    }

    return result;
}

// 基于高度和形状特征的精确聚类分析
ClusteringResult
performClustering(std::shared_ptr<open3d::geometry::PointCloud> cloud,
                  double eps, int min_points)
{
    ClusteringResult result;

    if (cloud->points_.empty())
    {
        return result;
    }

    WS_LOG_INFO("Open3D",
                "Starting clustering analysis on {} points...",
                cloud->points_.size());

    // 1. 基于高度的预分层
    auto bbox           = cloud->GetAxisAlignedBoundingBox();
    double min_z        = bbox.GetMinBound().z();
    double max_z        = bbox.GetMaxBound().z();
    double height_range = max_z - min_z;

    WS_LOG_INFO("Open3D",
                "Point cloud height range: {:.2f}m (from {:.2f}m to {:.2f}m)",
                height_range,
                min_z,
                max_z);

    // 根据高度特征调整聚类参数
    double adaptive_eps = eps;
    if (height_range > 50.0)
    {
        // 高层结构，增大聚类距离
        adaptive_eps = eps * 1.5;
        WS_LOG_INFO("Open3D",
                    "Detected high structures, adaptive eps: {:.2f} -> {:.2f}",
                    eps,
                    adaptive_eps);
    }

    // 2. 智能降采样以提高性能
    std::shared_ptr<open3d::geometry::PointCloud> processed_cloud = cloud;
    size_t original_size                                          = cloud->points_.size();

    // 如果点云过大，进行降采样
    if (original_size > 50000)
    {
        double voxel_size = 0.2; // 20cm体素大小
        WS_LOG_INFO("Open3D",
                    "Point cloud is large ({}), applying voxel "
                    "downsampling with size {:.2f}m...",
                    original_size,
                    voxel_size);

        auto downsample_start = std::chrono::high_resolution_clock::now();
        processed_cloud       = cloud->VoxelDownSample(voxel_size);
        auto downsample_end   = std::chrono::high_resolution_clock::now();
        auto downsample_duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                downsample_end - downsample_start);

        WS_LOG_INFO(
            "Open3D",
            "Downsampling completed in {}ms: {} -> {} points ({:.1f}% "
            "reduction)",
            downsample_duration.count(),
            original_size,
            processed_cloud->points_.size(),
            (1.0 - static_cast<double>(processed_cloud->points_.size()) /
                       original_size) *
                100.0);
    }

    // 根据降采样后的点云大小调整聚类参数
    size_t current_size     = processed_cloud->points_.size();
    int adjusted_min_points = min_points;

    if (current_size < original_size)
    {
        // 降采样后调整最小点数
        double reduction_ratio =
            static_cast<double>(current_size) / original_size;
        adjusted_min_points =
            std::max(10, static_cast<int>(min_points * reduction_ratio));
    }

    // 对于非常大的点云，进一步放宽参数
    if (current_size > 80000)
    {
        adaptive_eps *= 1.2;
        adjusted_min_points = std::max(adjusted_min_points, 15);
        WS_LOG_INFO("Open3D",
                    "Large point cloud detected, relaxing parameters");
    }

    WS_LOG_INFO(
        "Open3D",
        "DBSCAN parameters: eps={:.2f}, min_points={} (adjusted from {})",
        adaptive_eps,
        adjusted_min_points,
        min_points);

    // 3. 执行DBSCAN聚类
    WS_LOG_INFO("Open3D",
                "Executing DBSCAN clustering on {} points...",
                current_size);
    auto start_time = std::chrono::high_resolution_clock::now();

    result.clusterLabels =
        processed_cloud->ClusterDBSCAN(adaptive_eps, adjusted_min_points);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    WS_LOG_INFO("Open3D",
                "DBSCAN clustering completed in {}ms",
                duration.count());

    // 4. 获取聚类数量并统计
    int max_label = -1;
    if (!result.clusterLabels.empty())
    {
        max_label = *std::max_element(result.clusterLabels.begin(),
                                      result.clusterLabels.end());
    }

    // 统计噪声点
    int noise_points = std::count(result.clusterLabels.begin(),
                                  result.clusterLabels.end(),
                                  -1);

    WS_LOG_INFO("Open3D",
                "Raw clustering results: {} clusters, {} noise points",
                max_label + 1,
                noise_points);

    // 5. 为每个聚类创建点云并计算特征
    WS_LOG_INFO("Open3D", "Processing clusters and applying filters...");

    int valid_clusters  = 0;
    int filtered_small  = 0;
    int filtered_volume = 0;
    int filtered_points = 0;

    // 如果使用了降采样，需要映射回原始点云
    std::shared_ptr<open3d::geometry::PointCloud> source_cloud =
        (processed_cloud != cloud) ? cloud : processed_cloud;

    for (int i = 0; i <= max_label; ++i)
    {
        std::vector<size_t> cluster_indices;
        for (size_t j = 0; j < result.clusterLabels.size(); ++j)
        {
            if (result.clusterLabels[j] == i)
            {
                cluster_indices.push_back(j);
            }
        }

        if (!cluster_indices.empty())
        {
            auto cluster_cloud =
                processed_cloud->SelectByIndex(cluster_indices);

            // 计算聚类特征用于后续分类
            auto cluster_bbox = cluster_cloud->GetAxisAlignedBoundingBox();
            auto extent       = cluster_bbox.GetExtent();
            double height     = extent.z();
            double volume     = extent.x() * extent.y() * extent.z();

            // 调整过滤阈值（考虑降采样影响）
            double height_threshold = 2.0;
            double volume_threshold = 10.0;
            size_t points_threshold = adjusted_min_points;

            // 详细的过滤日志
            bool height_ok = height > height_threshold;
            bool volume_ok = volume > volume_threshold;
            bool points_ok = cluster_indices.size() >= points_threshold;

            if (!height_ok)
                filtered_small++;
            if (!volume_ok)
                filtered_volume++;
            if (!points_ok)
                filtered_points++;

            // 过滤掉太小的聚类
            if (height_ok && volume_ok && points_ok)
            {
                result.clusters.push_back(cluster_cloud);
                valid_clusters++;

                if (valid_clusters <= 10)
                { // 只显示前10个聚类的详细信息
                    WS_LOG_DEBUG("Open3D",
                                 "Cluster {}: {} points, height={:.2f}m, "
                                 "volume={:.2f}m³",
                                 i,
                                 cluster_indices.size(),
                                 height,
                                 volume);
                }
            }

            // 每处理20个聚类输出一次进度
            if ((i + 1) % 20 == 0 || i == max_label)
            {
                double progress =
                    static_cast<double>(i + 1) / (max_label + 1) * 100.0;
                WS_LOG_INFO("Open3D",
                            "Cluster processing progress: {:.1f}% ({}/{})",
                            progress,
                            i + 1,
                            max_label + 1);
            }
        }
    }

    // 输出过滤统计
    WS_LOG_INFO("Open3D", "Clustering filter results:");
    WS_LOG_INFO("Open3D", "  - Valid clusters: {}", valid_clusters);
    WS_LOG_INFO("Open3D",
                "  - Filtered (height < {:.1f}m): {}",
                2.0,
                filtered_small);
    WS_LOG_INFO("Open3D",
                "  - Filtered (volume < {:.1f}m³): {}",
                10.0,
                filtered_volume);
    WS_LOG_INFO("Open3D",
                "  - Filtered (points < {}): {}",
                adjusted_min_points,
                filtered_points);
    WS_LOG_INFO("Open3D", "  - Total processed: {}", max_label + 1);

    if (current_size != original_size)
    {
        WS_LOG_INFO("Open3D",
                    "  - Note: Results based on downsampled data ({} points)",
                    current_size);
    }

    return result;
}

// 精确的电力基础设施分类
InfrastructureClassification classifyInfrastructure(
    const std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& clusters,
    std::shared_ptr<open3d::geometry::PointCloud> ground_points)
{
    InfrastructureClassification result;
    result.groundPoints = ground_points;

    // 计算地面高度用于相对高度计算
    double ground_z = 0.0;
    if (ground_points && !ground_points->points_.empty())
    {
        auto ground_bbox = ground_points->GetAxisAlignedBoundingBox();
        ground_z         = ground_bbox.GetMaxBound().z();
    }

    // 存储候选电力塔和电力线
    std::vector<
        std::pair<std::shared_ptr<open3d::geometry::PointCloud>, double>>
        tower_candidates;
    std::vector<std::shared_ptr<open3d::geometry::PointCloud>>
        powerline_candidates;

    // 分析每个聚类
    for (const auto& cluster : clusters)
    {
        if (cluster->points_.empty())
            continue;

        auto bbox   = cluster->GetAxisAlignedBoundingBox();
        auto extent = bbox.GetExtent();
        auto center = bbox.GetCenter();

        double width           = extent.x();
        double depth           = extent.y();
        double height          = extent.z();
        double relative_height = center.z() - ground_z;
        double volume          = width * depth * height;
        double base_area       = width * depth;

        // 计算形状特征
        double aspect_ratio_xy =
            std::max(width, depth) / std::min(width, depth);
        double aspect_ratio_z = height / std::min(width, depth);
        double compactness    = volume / (width * depth * height);

        // 计算密度特征
        double point_density = cluster->points_.size() / volume;

        // 电力塔识别规则（更严格的条件）
        bool is_tower_candidate =
            (relative_height > 15.0 &&     // 相对地面高度超过15米
             height > 20.0 &&              // 绝对高度超过20米
             aspect_ratio_z > 2.0 &&       // 垂直结构（高度至少是底面的2倍）
             aspect_ratio_xy < 3.0 &&      // 底面相对方正
             base_area > 20.0 &&           // 足够的底面积
             volume > 400.0 &&             // 足够的体积
             point_density > 0.1 &&        // 合理的点密度
             cluster->points_.size() > 100 // 足够的点数
            );

        // 电力线识别规则（更精确的条件）
        bool is_powerline_candidate =
            (relative_height > 5.0 &&            // 相对地面高度超过5米
             height > 8.0 &&                     // 最小高度
             (aspect_ratio_xy > 5.0 ||           // 长条形结构
              (width > 30.0 || depth > 30.0)) && // 或者在某一方向很长
             volume < 500.0 &&                   // 体积不能太大（排除建筑物）
             volume > 5.0 &&                     // 体积不能太小
             cluster->points_.size() > 20 &&     // 最小点数
             cluster->points_.size() < 2000      // 最大点数（排除大型结构）
            );

        if (is_tower_candidate)
        {
            // 计算塔的评分（高度和密度权重）
            double tower_score =
                relative_height * 0.4 + volume * 0.0001 + point_density * 10.0;
            tower_candidates.push_back({cluster, tower_score});
        }
        else if (is_powerline_candidate)
        {
            powerline_candidates.push_back(cluster);
        }
    }

    // 选择最佳的电力塔候选
    if (!tower_candidates.empty())
    {
        // 按评分排序，选择最高分的作为电力塔
        std::sort(tower_candidates.begin(),
                  tower_candidates.end(),
                  [](const auto& a, const auto& b)
                  {
                      return a.second > b.second;
                  });

        result.towerPoints = tower_candidates[0].first;

        // 设置塔的包围盒
        auto tower_bbox = result.towerPoints->GetAxisAlignedBoundingBox();
        auto min_bound  = tower_bbox.GetMinBound();
        auto max_bound  = tower_bbox.GetMaxBound();
        result.towerBbox =
            math::BoundingBox{math::Vector3(static_cast<float>(min_bound.x()),
                                            static_cast<float>(min_bound.y()),
                                            static_cast<float>(min_bound.z())),
                              math::Vector3(static_cast<float>(max_bound.x()),
                                            static_cast<float>(max_bound.y()),
                                            static_cast<float>(max_bound.z()))};
    }

    // 进一步过滤电力线候选
    if (result.towerPoints && !powerline_candidates.empty())
    {
        auto tower_center =
            result.towerPoints->GetAxisAlignedBoundingBox().GetCenter();

        // 按照与塔的距离和高度特征进一步筛选电力线
        for (const auto& candidate : powerline_candidates)
        {
            auto line_center =
                candidate->GetAxisAlignedBoundingBox().GetCenter();
            double distance_to_tower =
                std::sqrt(std::pow(line_center.x() - tower_center.x(), 2) +
                          std::pow(line_center.y() - tower_center.y(), 2));

            // 电力线应该在塔附近但不能太近
            if (distance_to_tower > 5.0 && distance_to_tower < 200.0)
            {
                // 检查高度是否合理（电力线通常比塔低）
                if (line_center.z() < tower_center.z() &&
                    line_center.z() > ground_z + 8.0)
                {
                    result.powerLines.push_back(candidate);
                }
            }
        }
    }
    else
    {
        // 如果没有识别到塔，直接添加所有电力线候选
        result.powerLines = powerline_candidates;
    }

    return result;
}

// 电力线曲线拟合
std::vector<math::Vector3>
fitPowerLineCurve(std::shared_ptr<open3d::geometry::PointCloud> line_cloud)
{

    std::vector<math::Vector3> curve_points;

    if (line_cloud->points_.empty())
        return curve_points;

    // 简单的曲线拟合：按主要方向排序点，然后进行平滑
    auto bbox   = line_cloud->GetAxisAlignedBoundingBox();
    auto extent = bbox.GetExtent();

    // 确定主要方向（x或y）
    bool sort_by_x = extent.x() > extent.y();

    // 将点转换为我们的格式并排序
    std::vector<std::pair<double, math::Vector3>> sorted_points;
    for (const auto& point : line_cloud->points_)
    {
        math::Vector3 p(static_cast<float>(point.x()),
                        static_cast<float>(point.y()),
                        static_cast<float>(point.z()));
        double sort_key = sort_by_x ? point.x() : point.y();
        sorted_points.emplace_back(sort_key, p);
    }

    // 使用lambda进行排序
    std::sort(sorted_points.begin(),
              sorted_points.end(),
              [](const auto& a, const auto& b)
              {
                  return a.first < b.first;
              });

    // 简单的平滑处理：每隔几个点取一个代表点
    int step =
        std::max(1,
                 static_cast<int>(sorted_points.size() / 20)); // 最多20个控制点
    for (size_t i = 0; i < sorted_points.size(); i += step)
    {
        curve_points.push_back(sorted_points[i].second);
    }

    return curve_points;
}

// 生成简单的UUID字符串
std::string generateUUID(std::mt19937& gen)
{
    std::uniform_int_distribution<> hex_dis(0, 15);
    std::stringstream ss;

    // 格式: 8-4-4-4-12 (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx)
    for (int i = 0; i < 32; ++i)
    {
        if (i == 8 || i == 12 || i == 16 || i == 20)
        {
            ss << "-";
        }
        ss << std::hex << hex_dis(gen);
    }

    return ss.str();
}

// 生成模拟的电力线参数数据
void generatePowerLineParameters(
    InfrastructureClassification const& infrastructure,
    std::string const& filename, ecs::Commands commands)
{
    // 清空之前的参数
    World::powerLineParams.clear();

    // 设置处理过的文件名
    World::lastProcessedFile = filename;

    // 随机数生成器
    std::random_device rd;
    std::mt19937 gen(rd());

    // 为每条识别到的电力线生成模拟参数
    for (size_t i = 0; i < infrastructure.powerLines.size(); ++i)
    {
        const auto& power_line = infrastructure.powerLines[i];
        if (!power_line || power_line->points_.empty())
            continue;

        // 计算电力线的实际几何参数
        auto bbox      = power_line->GetAxisAlignedBoundingBox();
        auto extent    = bbox.GetExtent();
        auto center    = bbox.GetCenter();
        auto min_bound = bbox.GetMinBound();
        auto max_bound = bbox.GetMaxBound();

        double line_length = std::max(extent.x(), extent.y());
        double avg_height  = center.z();

        // 计算地面高度（用于相对高度计算）
        double ground_z = 0.0;
        if (infrastructure.groundPoints &&
            !infrastructure.groundPoints->points_.empty())
        {
            auto ground_bbox =
                infrastructure.groundPoints->GetAxisAlignedBoundingBox();
            ground_z = ground_bbox.GetMaxBound().z();
        }
        double relative_height = avg_height - ground_z;

        // 确定起点和终点坐标
        float startX, startY, startZ, endX, endY, endZ;
        if (extent.x() > extent.y())
        {
            // X方向更长，起点终点在X方向
            startX = static_cast<float>(min_bound.x());
            endX   = static_cast<float>(max_bound.x());
            startY = endY = static_cast<float>(center.y());
        }
        else
        {
            // Y方向更长，起点终点在Y方向
            startY = static_cast<float>(min_bound.y());
            endY   = static_cast<float>(max_bound.y());
            startX = endX = static_cast<float>(center.x());
        }
        startZ = endZ = static_cast<float>(avg_height);

        // 生成导线宽度（基于高度推断）
        float width; // 导线直径，单位mm
        if (relative_height > 40.0)
        {
            width = 35.0f + (gen() % 10); // 35-45mm直径
        }
        else if (relative_height > 25.0)
        {
            width = 25.0f + (gen() % 8); // 25-33mm直径
        }
        else if (relative_height > 15.0)
        {
            width = 18.0f + (gen() % 6); // 18-24mm直径
        }
        else
        {
            width = 12.0f + (gen() % 4); // 12-16mm直径
        }

        // 计算最大弧垂（基于线路长度和高度的经验公式）
        float maxSag =
            static_cast<float>(line_length * line_length /
                               (8.0 * std::max(relative_height, 10.0)));
        maxSag = std::max(0.5f, std::min(maxSag, 15.0f)); // 限制在合理范围内

        // 悬链线方程参数计算
        // y = a * cosh((x - h) / a) + k
        // 其中a控制曲线的陡峭程度，h是水平位移，k是垂直位移
        float catenaryA =
            static_cast<float>(relative_height / 3.0);           // 经验公式
        float catenaryH = static_cast<float>(line_length / 2.0); // 中点为原点
        float catenaryK = static_cast<float>(ground_z);          // 基准高度

        // 添加随机变化使数据更真实
        maxSag += std::uniform_real_distribution<float>(-0.5f, 0.5f)(gen);

        // 创建电力线参数
        PowerLineParameter param;
        param.id        = static_cast<int>(i + 1);
        param.name      = generateUUID(gen);
        param.startX    = startX;
        param.startY    = startY;
        param.startZ    = startZ;
        param.endX      = endX;
        param.endY      = endY;
        param.endZ      = endZ;
        param.length    = static_cast<float>(line_length);
        param.width     = width;
        param.maxSag    = std::max(0.1f, maxSag);
        param.catenaryA = catenaryA;
        param.catenaryH = catenaryH;
        param.catenaryK = catenaryK;

        World::powerLineParams.push_back(param);

        WS_LOG_INFO("PowerLine",
                    "Generated parameters for line {}: {:.1f}m length, "
                    "{:.1f}mm width",
                    i + 1,
                    param.length,
                    param.width);
    }

    // 如果没有检测到电力线，生成随机数量的示例数据（8-20条）
    if (World::powerLineParams.empty())
    {
        WS_LOG_WARN("PowerLine",
                    "No power lines detected, generating sample parameters");

        // 生成8-20条示例电力线
        std::uniform_int_distribution<int> count_dist(8, 20);
        int sample_count = count_dist(gen);

        for (int i = 0; i < sample_count; ++i)
        {
            PowerLineParameter param;
            param.id   = i + 1;
            param.name = generateUUID(gen);

            // 生成随机坐标
            param.startX =
                std::uniform_real_distribution<float>(-100.0f, 100.0f)(gen);
            param.startY =
                std::uniform_real_distribution<float>(-100.0f, 100.0f)(gen);
            param.startZ =
                std::uniform_real_distribution<float>(20.0f, 50.0f)(gen);
            param.endX =
                param.startX +
                std::uniform_real_distribution<float>(50.0f, 200.0f)(gen);
            param.endY =
                param.startY +
                std::uniform_real_distribution<float>(-20.0f, 20.0f)(gen);
            param.endZ =
                param.startZ +
                std::uniform_real_distribution<float>(-5.0f, 5.0f)(gen);

            param.length = std::sqrt(std::pow(param.endX - param.startX, 2) +
                                     std::pow(param.endY - param.startY, 2));
            param.width  = std::uniform_real_distribution<float>(12.0f, 45.0f)(
                gen); // 12-45mm
            param.maxSag =
                std::uniform_real_distribution<float>(1.0f,
                                                      15.0f)(gen); // 1-15m
            param.catenaryA = param.startZ / 3.0f;
            param.catenaryH = param.length / 2.0f;
            param.catenaryK = 0.0f;

            World::powerLineParams.push_back(param);
        }
    }

    WS_LOG_INFO("PowerLine",
                "Generated {} power line parameters",
                World::powerLineParams.size());

    // 触发弹出窗口

    commands.getResource<LayoutData>()->showPowerLineParamsPopup = true;
}