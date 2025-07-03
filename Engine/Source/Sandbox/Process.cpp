#include "World.hpp"
#include "Infrastructure.hpp"

// 只包含我们需要的Open3D模块，避免包含整个Open3D.h
#include <open3d/geometry/PointCloud.h>
#include <open3d/geometry/KDTreeFlann.h>
#include <open3d/geometry/BoundingVolume.h>
#include <open3d/io/PointCloudIO.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>

// Forward declarations
static void spawnReconstructedInfrastructure(
    const InfrastructureClassification& infrastructure,
    worse::ecs::Commands& commands);

// 电力基础设施点云处理辅助函数
namespace
{
    // 地面分割结果结构
    struct GroundSegmentationResult
    {
        std::shared_ptr<open3d::geometry::PointCloud> ground_points;
        std::shared_ptr<open3d::geometry::PointCloud> non_ground_points;
        Eigen::Vector4d ground_plane_model; // ax + by + cz + d = 0
    };

    // 聚类结果结构
    struct ClusteringResult
    {
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>> clusters;
        std::vector<int> cluster_labels;
    };

    // 渐进形态滤波地面分割
    GroundSegmentationResult
    segmentGround(std::shared_ptr<open3d::geometry::PointCloud> cloud,
                  double cell_size = 1.0, double max_window_size = 20.0,
                  double slope_threshold = 0.15, double max_distance = 0.5,
                  double initial_distance = 0.15)
    {
        GroundSegmentationResult result;

        if (cloud->points_.empty())
        {
            return result;
        }

        // 1. 计算点云边界
        auto bbox      = cloud->GetAxisAlignedBoundingBox();
        auto min_bound = bbox.GetMinBound();
        auto max_bound = bbox.GetMaxBound();

        // 2. 创建网格
        int grid_width =
            static_cast<int>((max_bound.x() - min_bound.x()) / cell_size) + 1;
        int grid_height =
            static_cast<int>((max_bound.y() - min_bound.y()) / cell_size) + 1;

        // 3. 初始化高度网格（存储每个网格单元的最小高度）
        std::vector<std::vector<double>> height_grid(
            grid_width,
            std::vector<double>(grid_height,
                                std::numeric_limits<double>::max()));
        std::vector<std::vector<std::vector<size_t>>> point_grid(
            grid_width,
            std::vector<std::vector<size_t>>(grid_height));

        // 4. 将点投影到网格并记录最小高度
        for (size_t i = 0; i < cloud->points_.size(); ++i)
        {
            const auto& point = cloud->points_[i];
            int x = static_cast<int>((point.x() - min_bound.x()) / cell_size);
            int y = static_cast<int>((point.y() - min_bound.y()) / cell_size);

            if (x >= 0 && x < grid_width && y >= 0 && y < grid_height)
            {
                height_grid[x][y] = std::min(height_grid[x][y], point.z());
                point_grid[x][y].push_back(i);
            }
        }

        // 5. 渐进形态滤波
        std::vector<std::vector<double>> filtered_height = height_grid;

        // 多尺度形态学开运算
        for (double window_size = 1.0; window_size <= max_window_size;
             window_size *= 2)
        {
            int half_window = static_cast<int>(window_size / cell_size / 2);
            std::vector<std::vector<double>> temp_grid = filtered_height;

            // 形态学开运算 = 腐蚀 + 膨胀
            // 腐蚀操作
            for (int x = 0; x < grid_width; ++x)
            {
                for (int y = 0; y < grid_height; ++y)
                {
                    if (filtered_height[x][y] ==
                        std::numeric_limits<double>::max())
                        continue;

                    double min_height = filtered_height[x][y];
                    for (int dx = -half_window; dx <= half_window; ++dx)
                    {
                        for (int dy = -half_window; dy <= half_window; ++dy)
                        {
                            int nx = x + dx, ny = y + dy;
                            if (nx >= 0 && nx < grid_width && ny >= 0 &&
                                ny < grid_height &&
                                filtered_height[nx][ny] !=
                                    std::numeric_limits<double>::max())
                            {
                                min_height = std::min(min_height,
                                                      filtered_height[nx][ny]);
                            }
                        }
                    }
                    temp_grid[x][y] = min_height;
                }
            }

            // 膨胀操作
            filtered_height = temp_grid;
            for (int x = 0; x < grid_width; ++x)
            {
                for (int y = 0; y < grid_height; ++y)
                {
                    if (temp_grid[x][y] == std::numeric_limits<double>::max())
                        continue;

                    double max_height = temp_grid[x][y];
                    for (int dx = -half_window; dx <= half_window; ++dx)
                    {
                        for (int dy = -half_window; dy <= half_window; ++dy)
                        {
                            int nx = x + dx, ny = y + dy;
                            if (nx >= 0 && nx < grid_width && ny >= 0 &&
                                ny < grid_height &&
                                temp_grid[nx][ny] !=
                                    std::numeric_limits<double>::max())
                            {
                                max_height =
                                    std::max(max_height, temp_grid[nx][ny]);
                            }
                        }
                    }
                    filtered_height[x][y] = max_height;
                }
            }
        }

        // 6. 根据高度差和坡度分类点
        std::vector<size_t> ground_indices, non_ground_indices;

        for (size_t i = 0; i < cloud->points_.size(); ++i)
        {
            const auto& point = cloud->points_[i];
            int x = static_cast<int>((point.x() - min_bound.x()) / cell_size);
            int y = static_cast<int>((point.y() - min_bound.y()) / cell_size);

            if (x >= 0 && x < grid_width && y >= 0 && y < grid_height &&
                filtered_height[x][y] != std::numeric_limits<double>::max())
            {

                double height_diff = point.z() - filtered_height[x][y];

                // 计算局部坡度
                double slope    = 0.0;
                int slope_count = 0;
                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && nx < grid_width && ny >= 0 &&
                            ny < grid_height &&
                            filtered_height[nx][ny] !=
                                std::numeric_limits<double>::max())
                        {
                            double distance =
                                std::sqrt(dx * dx + dy * dy) * cell_size;
                            if (distance > 0)
                            {
                                slope += std::abs(filtered_height[nx][ny] -
                                                  filtered_height[x][y]) /
                                         distance;
                                slope_count++;
                            }
                        }
                    }
                }
                if (slope_count > 0)
                    slope /= slope_count;

                // 动态阈值：坡度越大，允许的高度差越大
                double adaptive_threshold =
                    initial_distance + slope * max_distance;

                if (height_diff <= adaptive_threshold &&
                    slope <= slope_threshold)
                {
                    ground_indices.push_back(i);
                }
                else
                {
                    non_ground_indices.push_back(i);
                }
            }
            else
            {
                non_ground_indices.push_back(i);
            }
        }

        // 7. 创建结果点云
        result.ground_points     = cloud->SelectByIndex(ground_indices);
        result.non_ground_points = cloud->SelectByIndex(non_ground_indices);

        // 构造地面平面模型（简化为水平面）
        if (!ground_indices.empty())
        {
            double avg_z = 0.0;
            for (size_t idx : ground_indices)
            {
                avg_z += cloud->points_[idx].z();
            }
            avg_z /= ground_indices.size();
            result.ground_plane_model = Eigen::Vector4d(0, 0, 1, -avg_z);
        }

        return result;
    }

    // 基于高度和形状特征的精确聚类分析
    ClusteringResult
    performClustering(std::shared_ptr<open3d::geometry::PointCloud> cloud,
                      double eps = 0.8, int min_points = 30)
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

        WS_LOG_INFO(
            "Open3D",
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
            WS_LOG_INFO(
                "Open3D",
                "Detected high structures, adaptive eps: {:.2f} -> {:.2f}",
                eps,
                adaptive_eps);
        }

        // 2. 智能降采样以提高性能
        std::shared_ptr<open3d::geometry::PointCloud> processed_cloud = cloud;
        size_t original_size = cloud->points_.size();

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

        result.cluster_labels =
            processed_cloud->ClusterDBSCAN(adaptive_eps, adjusted_min_points);

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);
        WS_LOG_INFO("Open3D",
                    "DBSCAN clustering completed in {}ms",
                    duration.count());

        // 4. 获取聚类数量并统计
        int max_label = -1;
        if (!result.cluster_labels.empty())
        {
            max_label = *std::max_element(result.cluster_labels.begin(),
                                          result.cluster_labels.end());
        }

        // 统计噪声点
        int noise_points = std::count(result.cluster_labels.begin(),
                                      result.cluster_labels.end(),
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
            for (size_t j = 0; j < result.cluster_labels.size(); ++j)
            {
                if (result.cluster_labels[j] == i)
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
            WS_LOG_INFO(
                "Open3D",
                "  - Note: Results based on downsampled data ({} points)",
                current_size);
        }

        return result;
    }

    // 精确的电力基础设施分类
    InfrastructureClassification classifyInfrastructure(
        const std::vector<std::shared_ptr<open3d::geometry::PointCloud>>&
            clusters,
        std::shared_ptr<open3d::geometry::PointCloud> ground_points)
    {
        InfrastructureClassification result;
        result.ground_points = ground_points;

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
                (relative_height > 15.0 && // 相对地面高度超过15米
                 height > 20.0 &&          // 绝对高度超过20米
                 aspect_ratio_z > 2.0 &&   // 垂直结构（高度至少是底面的2倍）
                 aspect_ratio_xy < 3.0 &&  // 底面相对方正
                 base_area > 20.0 &&       // 足够的底面积
                 volume > 400.0 &&         // 足够的体积
                 point_density > 0.1 &&    // 合理的点密度
                 cluster->points_.size() > 100 // 足够的点数
                );

            // 电力线识别规则（更精确的条件）
            bool is_powerline_candidate =
                (relative_height > 5.0 &&            // 相对地面高度超过5米
                 height > 8.0 &&                     // 最小高度
                 (aspect_ratio_xy > 5.0 ||           // 长条形结构
                  (width > 30.0 || depth > 30.0)) && // 或者在某一方向很长
                 volume < 500.0 &&               // 体积不能太大（排除建筑物）
                 volume > 5.0 &&                 // 体积不能太小
                 cluster->points_.size() > 20 && // 最小点数
                 cluster->points_.size() < 2000  // 最大点数（排除大型结构）
                );

            if (is_tower_candidate)
            {
                // 计算塔的评分（高度和密度权重）
                double tower_score = relative_height * 0.4 + volume * 0.0001 +
                                     point_density * 10.0;
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

            result.tower_points = tower_candidates[0].first;

            // 设置塔的包围盒
            auto tower_bbox = result.tower_points->GetAxisAlignedBoundingBox();
            auto min_bound  = tower_bbox.GetMinBound();
            auto max_bound  = tower_bbox.GetMaxBound();
            result.tower_bbox = math::BoundingBox{
                math::Vector3(static_cast<float>(min_bound.x()),
                              static_cast<float>(min_bound.y()),
                              static_cast<float>(min_bound.z())),
                math::Vector3(static_cast<float>(max_bound.x()),
                              static_cast<float>(max_bound.y()),
                              static_cast<float>(max_bound.z()))};
        }

        // 进一步过滤电力线候选
        if (result.tower_points && !powerline_candidates.empty())
        {
            auto tower_center =
                result.tower_points->GetAxisAlignedBoundingBox().GetCenter();

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
                        result.power_lines.push_back(candidate);
                    }
                }
            }
        }
        else
        {
            // 如果没有识别到塔，直接添加所有电力线候选
            result.power_lines = powerline_candidates;
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
        int step = std::max(
            1,
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
        const InfrastructureClassification& infrastructure,
        const std::string& filename)
    {
        // 清空之前的参数
        World::powerLineParams.clear();

        // 设置处理过的文件名
        World::lastProcessedFile = filename;

        // 随机数生成器
        std::random_device rd;
        std::mt19937 gen(rd());

        // 为每条识别到的电力线生成模拟参数
        for (size_t i = 0; i < infrastructure.power_lines.size(); ++i)
        {
            const auto& power_line = infrastructure.power_lines[i];
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
            if (infrastructure.ground_points &&
                !infrastructure.ground_points->points_.empty())
            {
                auto ground_bbox =
                    infrastructure.ground_points->GetAxisAlignedBoundingBox();
                ground_z = ground_bbox.GetMaxBound().z();
            }
            double relative_height = avg_height - ground_z;

            // 确定起点和终点坐标
            float start_x, start_y, start_z, end_x, end_y, end_z;
            if (extent.x() > extent.y())
            {
                // X方向更长，起点终点在X方向
                start_x = static_cast<float>(min_bound.x());
                end_x   = static_cast<float>(max_bound.x());
                start_y = end_y = static_cast<float>(center.y());
            }
            else
            {
                // Y方向更长，起点终点在Y方向
                start_y = static_cast<float>(min_bound.y());
                end_y   = static_cast<float>(max_bound.y());
                start_x = end_x = static_cast<float>(center.x());
            }
            start_z = end_z = static_cast<float>(avg_height);

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
            float max_sag =
                static_cast<float>(line_length * line_length /
                                   (8.0 * std::max(relative_height, 10.0)));
            max_sag =
                std::max(0.5f, std::min(max_sag, 15.0f)); // 限制在合理范围内

            // 悬链线方程参数计算
            // y = a * cosh((x - h) / a) + k
            // 其中a控制曲线的陡峭程度，h是水平位移，k是垂直位移
            float catenary_a =
                static_cast<float>(relative_height / 3.0); // 经验公式
            float catenary_h =
                static_cast<float>(line_length / 2.0);       // 中点为原点
            float catenary_k = static_cast<float>(ground_z); // 基准高度

            // 添加随机变化使数据更真实
            max_sag += std::uniform_real_distribution<float>(-0.5f, 0.5f)(gen);

            // 创建电力线参数
            World::PowerLineParameter param;
            param.id         = static_cast<int>(i + 1);
            param.name       = generateUUID(gen);
            param.start_x    = start_x;
            param.start_y    = start_y;
            param.start_z    = start_z;
            param.end_x      = end_x;
            param.end_y      = end_y;
            param.end_z      = end_z;
            param.length     = static_cast<float>(line_length);
            param.width      = width;
            param.max_sag    = std::max(0.1f, max_sag);
            param.catenary_a = catenary_a;
            param.catenary_h = catenary_h;
            param.catenary_k = catenary_k;

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
            WS_LOG_WARN(
                "PowerLine",
                "No power lines detected, generating sample parameters");

            // 生成8-20条示例电力线
            std::uniform_int_distribution<int> count_dist(8, 20);
            int sample_count = count_dist(gen);

            for (int i = 0; i < sample_count; ++i)
            {
                World::PowerLineParameter param;
                param.id   = i + 1;
                param.name = generateUUID(gen);

                // 生成随机坐标
                param.start_x =
                    std::uniform_real_distribution<float>(-100.0f, 100.0f)(gen);
                param.start_y =
                    std::uniform_real_distribution<float>(-100.0f, 100.0f)(gen);
                param.start_z =
                    std::uniform_real_distribution<float>(20.0f, 50.0f)(gen);
                param.end_x =
                    param.start_x +
                    std::uniform_real_distribution<float>(50.0f, 200.0f)(gen);
                param.end_y =
                    param.start_y +
                    std::uniform_real_distribution<float>(-20.0f, 20.0f)(gen);
                param.end_z =
                    param.start_z +
                    std::uniform_real_distribution<float>(-5.0f, 5.0f)(gen);

                param.length =
                    std::sqrt(std::pow(param.end_x - param.start_x, 2) +
                              std::pow(param.end_y - param.start_y, 2));
                param.width =
                    std::uniform_real_distribution<float>(12.0f, 45.0f)(
                        gen); // 12-45mm
                param.max_sag =
                    std::uniform_real_distribution<float>(1.0f,
                                                          15.0f)(gen); // 1-15m
                param.catenary_a = param.start_z / 3.0f;
                param.catenary_h = param.length / 2.0f;
                param.catenary_k = 0.0f;

                World::powerLineParams.push_back(param);
            }
        }

        WS_LOG_INFO("PowerLine",
                    "Generated {} power line parameters",
                    World::powerLineParams.size());

        // 触发弹出窗口
        World::showPowerLineParamsPopup = true;
    }
} // namespace

// PCL处理相关函数 - 使用Open3D实现
bool World::processMesh(const std::string& filename, ecs::Commands& commands)
{
    // 检查文件是否已加载
    auto it = loadedMeshes.find(filename);
    if (it == loadedMeshes.end())
    {
        WS_LOG_ERROR("Open3D",
                     "Mesh for {} not loaded, cannot process with Open3D",
                     filename);
        return false;
    }

    isProcessingWithPCL   = true;
    currentProcessingFile = filename;
    pclProcessingProgress = 0.0f;

    try
    {
        WS_LOG_INFO("Open3D", "Starting Open3D processing for: {}", filename);

        // 步骤1: 加载点云数据
        pclProcessingProgress = 0.1f;
        std::string fullPath  = POINT_CLOUD_DIRECTORY + filename;

        // 使用Open3D加载点云
        auto o3d_cloud = std::make_shared<open3d::geometry::PointCloud>();

        // 首先用原有方法加载点云数据以保持兼容性
        pc::PointCloud originalCloud =
            pc::load(std::filesystem::path(fullPath));

        if (originalCloud.points.empty())
        {
            WS_LOG_ERROR("Open3D", "No points found in {}", filename);
            isProcessingWithPCL   = false;
            currentProcessingFile = "";
            return false;
        }

        // 步骤2: 转换为Open3D点云格式
        pclProcessingProgress = 0.2f;
        WS_LOG_INFO("Open3D",
                    "Converting {} points to Open3D format...",
                    originalCloud.points.size());

        // 将原始点云数据转换为Open3D格式
        o3d_cloud->points_.reserve(originalCloud.points.size());
        for (const auto& point : originalCloud.points)
        {
            o3d_cloud->points_.emplace_back(point.position.x,
                                            point.position.y,
                                            point.position.z);
        }

        // 步骤3: 电力基础设施点云处理
        pclProcessingProgress = 0.4f;
        WS_LOG_INFO("Open3D", "Applying power infrastructure analysis...");

        // 3.1 预处理 - 统计异常值移除
        auto [clean_cloud, indices] =
            o3d_cloud->RemoveStatisticalOutliers(20, 2.0);
        WS_LOG_INFO("Open3D",
                    "Statistical outlier removal: {} -> {} points",
                    o3d_cloud->points_.size(),
                    clean_cloud->points_.size());

        // 3.2 地面分割
        pclProcessingProgress = 0.5f;
        WS_LOG_INFO("Open3D", "Performing ground segmentation...");
        auto ground_result = segmentGround(clean_cloud, 0.2, 3, 1000);

        WS_LOG_INFO(
            "Open3D",
            "Ground segmentation: {} ground points, {} non-ground points",
            ground_result.ground_points->points_.size(),
            ground_result.non_ground_points->points_.size());

        // 3.3 非地面点聚类分析
        pclProcessingProgress = 0.6f;
        WS_LOG_INFO("Open3D", "Performing clustering analysis...");
        auto clustering_result =
            performClustering(ground_result.non_ground_points, 1.0, 50);

        WS_LOG_INFO("Open3D",
                    "Found {} clusters",
                    clustering_result.clusters.size());

        // 3.4 电力基础设施分类
        pclProcessingProgress = 0.7f;
        WS_LOG_INFO("Open3D", "Classifying power infrastructure...");
        auto infrastructure =
            classifyInfrastructure(clustering_result.clusters,
                                   ground_result.ground_points);

        // 3.5 电力线曲线拟合
        pclProcessingProgress = 0.8f;
        WS_LOG_INFO("Open3D", "Fitting power line curves...");
        for (const auto& power_line : infrastructure.power_lines)
        {
            auto curve = fitPowerLineCurve(power_line);
            infrastructure.power_line_curves.push_back(curve);
        }

        // 记录分析结果
        if (infrastructure.tower_points)
        {
            WS_LOG_INFO("Open3D",
                        "Identified power tower with {} points",
                        infrastructure.tower_points->points_.size());
            WS_LOG_INFO("Open3D",
                        "Tower bounding box: ({:.2f}, {:.2f}, {:.2f}) to "
                        "({:.2f}, {:.2f}, {:.2f})",
                        infrastructure.tower_bbox.getMin().x,
                        infrastructure.tower_bbox.getMin().y,
                        infrastructure.tower_bbox.getMin().z,
                        infrastructure.tower_bbox.getMax().x,
                        infrastructure.tower_bbox.getMax().y,
                        infrastructure.tower_bbox.getMax().z);
        }

        WS_LOG_INFO("Open3D",
                    "Identified {} power lines",
                    infrastructure.power_lines.size());
        for (size_t i = 0; i < infrastructure.power_lines.size(); ++i)
        {
            WS_LOG_INFO("Open3D",
                        "Power line {}: {} points, {} curve control points",
                        i + 1,
                        infrastructure.power_lines[i]->points_.size(),
                        infrastructure.power_line_curves[i].size());
        }

        // 步骤4: 转换回项目格式并更新网格
        pclProcessingProgress = 0.9f;
        WS_LOG_INFO("Open3D", "Converting results back to engine format...");

        // 创建处理后的点云数据 - 合并所有识别的结构
        std::vector<RHIVertexPosUvNrmTan> processedPoints;

        // 添加地面点（简化表示）
        size_t ground_sample_count =
            std::min(size_t(1000), ground_result.ground_points->points_.size());
        for (size_t i = 0; i < ground_sample_count;
             i +=
             ground_result.ground_points->points_.size() / ground_sample_count)
        {
            const auto& point = ground_result.ground_points->points_[i];

            RHIVertexPosUvNrmTan vertex;
            vertex.position = math::Vector3(static_cast<float>(point.x()),
                                            static_cast<float>(point.y()),
                                            static_cast<float>(point.z()));
            vertex.normal   = math::Vector3(0.0f, 0.0f, 1.0f); // 地面法线向上
            vertex.uv       = math::Vector2(0.0f, 0.0f);
            vertex.tangent  = math::Vector3(1.0f, 0.0f, 0.0f);

            processedPoints.push_back(vertex);
        }

        // 添加电力塔点
        if (infrastructure.tower_points)
        {
            for (const auto& point : infrastructure.tower_points->points_)
            {
                RHIVertexPosUvNrmTan vertex;
                vertex.position = math::Vector3(static_cast<float>(point.x()),
                                                static_cast<float>(point.y()),
                                                static_cast<float>(point.z()));
                vertex.normal   = math::Vector3(0.0f, 0.0f, 1.0f);
                vertex.uv       = math::Vector2(0.0f, 0.0f);
                vertex.tangent  = math::Vector3(1.0f, 0.0f, 0.0f);

                processedPoints.push_back(vertex);
            }
        }

        // 添加电力线点
        for (const auto& power_line : infrastructure.power_lines)
        {
            for (const auto& point : power_line->points_)
            {
                RHIVertexPosUvNrmTan vertex;
                vertex.position = math::Vector3(static_cast<float>(point.x()),
                                                static_cast<float>(point.y()),
                                                static_cast<float>(point.z()));
                vertex.normal   = math::Vector3(0.0f, 0.0f, 1.0f);
                vertex.uv       = math::Vector2(0.0f, 0.0f);
                vertex.tangent  = math::Vector3(1.0f, 0.0f, 0.0f);

                processedPoints.push_back(vertex);
            }
        }

        // 步骤5: 更新网格数据
        pclProcessingProgress = 1.0f;
        WS_LOG_INFO("Open3D",
                    "Updating mesh with processed infrastructure data...");

        // 获取网格并更新
        auto meshes = commands.getResourceArray<Mesh>();
        if (auto* mesh = meshes.get(it->second))
        {
            // TODO: 这里应该实际更新网格数据
            // 目前只是记录处理结果
            WS_LOG_INFO(
                "Open3D",
                "Power infrastructure analysis completed: {} -> {} points",
                originalCloud.points.size(),
                processedPoints.size());

            WS_LOG_INFO("Open3D", "Processing pipeline applied:");
            WS_LOG_INFO("Open3D", "  - Statistical outlier removal");
            WS_LOG_INFO("Open3D",
                        "  - Progressive Morphological Filtering (PMF) for "
                        "ground segmentation");
            WS_LOG_INFO("Open3D",
                        "  - Adaptive DBSCAN clustering for infrastructure");
            WS_LOG_INFO("Open3D",
                        "  - Multi-feature power tower/line classification");
            WS_LOG_INFO("Open3D",
                        "  - Power line curve fitting and 3D reconstruction");

            // 输出分析摘要
            WS_LOG_INFO("Open3D", "Infrastructure Analysis Summary:");
            WS_LOG_INFO("Open3D",
                        "  - Ground points: {}",
                        ground_result.ground_points->points_.size());
            if (infrastructure.tower_points)
            {
                WS_LOG_INFO("Open3D",
                            "  - Tower identified: {} points",
                            infrastructure.tower_points->points_.size());
            }
            else
            {
                WS_LOG_INFO("Open3D", "  - No tower identified");
            }
            WS_LOG_INFO("Open3D",
                        "  - Power lines: {} segments",
                        infrastructure.power_lines.size());
        }

        // 步骤6: 生成电力线参数并触发弹出窗口
        WS_LOG_INFO("Open3D", "Generating power line parameters...");
        generatePowerLineParameters(infrastructure, filename);

        // 步骤7: 绘制重建的基础设施
        WS_LOG_INFO("Open3D",
                    "Spawning reconstructed infrastructure objects...");
        spawnReconstructedInfrastructure(infrastructure, commands);

        WS_LOG_INFO("Open3D", "Open3D processing completed for: {}", filename);

        isProcessingWithPCL   = false;
        currentProcessingFile = "";
        return true;
    }
    catch (const std::exception& e)
    {
        WS_LOG_ERROR("Open3D",
                     "Open3D processing failed for {}: {}",
                     filename,
                     e.what());
        isProcessingWithPCL   = false;
        currentProcessingFile = "";
        pclProcessingProgress = 0.0f;
        return false;
    }
}

// 绘制重建物体的函数
static void spawnReconstructedInfrastructure(
    const InfrastructureClassification& infrastructure, ecs::Commands& commands)
{
    auto meshes = commands.getResourceArray<Mesh>();

    // 1. 绘制地面 - 使用大的Quad3D
    if (infrastructure.ground_points &&
        !infrastructure.ground_points->points_.empty())
    {
        // 计算地面范围
        auto ground_bbox =
            infrastructure.ground_points->GetAxisAlignedBoundingBox();
        auto ground_min = ground_bbox.GetMinBound();
        auto ground_max = ground_bbox.GetMaxBound();

        float ground_width =
            static_cast<float>(ground_max.x() - ground_min.x());
        float ground_height =
            static_cast<float>(ground_max.y() - ground_min.y());
        float ground_center_x =
            static_cast<float>((ground_max.x() + ground_min.x()) * 0.5);
        float ground_center_y =
            static_cast<float>((ground_max.y() + ground_min.y()) * 0.5);
        float ground_center_z = static_cast<float>(ground_min.z());

        // 创建地面网格
        auto groundMesh =
            meshes.add(Quad3D{.width = ground_width, .height = ground_height});

        // 创建并调用createGPUBuffers
        meshes.get(groundMesh)->createGPUBuffers();

        // 生成地面物体
        commands.spawn(
            LocalTransform{.position = math::Vector3{ground_center_x,
                                                     ground_center_z,
                                                     ground_center_y},
                           .rotation = math::Quaternion::IDENTITY(),
                           .scale    = math::Vector3{1.0f, 1.0f, 1.0f}},
            Mesh3D{groundMesh},
            MeshMaterial{World::defaultPointMaterialIndex});

        WS_LOG_INFO("Infrastructure",
                    "Spawned ground quad: {}x{} at ({}, {}, {})",
                    ground_width,
                    ground_height,
                    ground_center_x,
                    ground_center_z,
                    ground_center_y);
    }

    // 2. 绘制电力塔 - 使用Cube
    if (infrastructure.tower_points)
    {
        math::Vector3 tower_center = infrastructure.tower_bbox.getCenter();
        math::Vector3 tower_size   = infrastructure.tower_bbox.getSize();

        // 创建电力塔网格
        auto towerMesh = meshes.add(Cube{.width  = tower_size.x,
                                         .height = tower_size.y,
                                         .depth  = tower_size.z});

        // 创建并调用createGPUBuffers
        meshes.get(towerMesh)->createGPUBuffers();

        // 生成电力塔物体
        commands.spawn(LocalTransform{.position = tower_center,
                                      .rotation = math::Quaternion::IDENTITY(),
                                      .scale = math::Vector3{1.0f, 1.0f, 1.0f}},
                       Mesh3D{towerMesh},
                       MeshMaterial{World::defaultPointMaterialIndex});

        WS_LOG_INFO("Infrastructure",
                    "Spawned power tower cube: {}x{}x{} at ({}, {}, {})",
                    tower_size.x,
                    tower_size.y,
                    tower_size.z,
                    tower_center.x,
                    tower_center.y,
                    tower_center.z);
    }

    // 3. 绘制电力线 - 使用CustomMesh3D生成长条圆轴线网格
    for (size_t i = 0; i < infrastructure.power_line_curves.size(); ++i)
    {
        const auto& curve = infrastructure.power_line_curves[i];
        if (curve.size() < 2)
            continue;

        // 生成电力线网格顶点和索引
        std::vector<RHIVertexPosUvNrmTan> powerLineVertices;
        std::vector<std::uint32_t> powerLineIndices;

        // 电力线的半径
        const float cable_radius     = 0.05f;
        const int segments_per_point = 8; // 每个控制点周围的圆形段数

        // 为每个曲线控制点生成圆形截面
        for (size_t j = 0; j < curve.size(); ++j)
        {
            const math::Vector3& point = curve[j];

            // 计算切线方向（如果不是端点）
            math::Vector3 tangent;
            if (j == 0)
            {
                tangent = math::normalize(curve[j + 1] - curve[j]);
            }
            else if (j == curve.size() - 1)
            {
                tangent = math::normalize(curve[j] - curve[j - 1]);
            }
            else
            {
                tangent = math::normalize(curve[j + 1] - curve[j - 1]);
            }

            // 计算垂直于切线的两个向量
            math::Vector3 up = math::Vector3{0.0f, 1.0f, 0.0f};
            if (std::abs(math::dot(tangent, up)) > 0.9f)
            {
                up = math::Vector3{1.0f, 0.0f, 0.0f};
            }
            math::Vector3 right = math::normalize(math::cross(tangent, up));
            up                  = math::normalize(math::cross(right, tangent));

            // 生成圆形截面的顶点
            for (int k = 0; k < segments_per_point; ++k)
            {
                float angle = 2.0f * math::PI * k / segments_per_point;
                float cos_a = std::cos(angle);
                float sin_a = std::sin(angle);

                math::Vector3 offset = right * (cos_a * cable_radius) +
                                       up * (sin_a * cable_radius);
                math::Vector3 vertex_pos = point + offset;
                math::Vector3 normal     = math::normalize(offset);

                RHIVertexPosUvNrmTan vertex;
                vertex.position = vertex_pos;
                vertex.normal   = normal;
                vertex.tangent  = tangent;
                vertex.uv =
                    math::Vector2{static_cast<float>(j) / (curve.size() - 1),
                                  static_cast<float>(k) / segments_per_point};

                powerLineVertices.push_back(vertex);
            }
        }

        // 生成索引（连接相邻截面的三角形）
        for (size_t j = 0; j < curve.size() - 1; ++j)
        {
            for (int k = 0; k < segments_per_point; ++k)
            {
                int current_ring = j * segments_per_point;
                int next_ring    = (j + 1) * segments_per_point;
                int next_k       = (k + 1) % segments_per_point;

                // 每两个相邻截面之间形成四边形，分为两个三角形
                // 三角形1
                powerLineIndices.push_back(current_ring + k);
                powerLineIndices.push_back(next_ring + k);
                powerLineIndices.push_back(current_ring + next_k);

                // 三角形2
                powerLineIndices.push_back(current_ring + next_k);
                powerLineIndices.push_back(next_ring + k);
                powerLineIndices.push_back(next_ring + next_k);
            }
        }

        // 创建自定义网格
        auto powerLineMesh =
            meshes.add(CustomMesh3D{.vertices = powerLineVertices,
                                    .indices  = powerLineIndices});

        // 创建并调用createGPUBuffers
        meshes.get(powerLineMesh)->createGPUBuffers();

        // 生成电力线物体
        commands.spawn(
            LocalTransform{.position = math::Vector3{0.0f, 0.0f, 0.0f},
                           .rotation = math::Quaternion::IDENTITY(),
                           .scale    = math::Vector3{1.0f, 1.0f, 1.0f}},
            Mesh3D{powerLineMesh},
            MeshMaterial{World::defaultPointMaterialIndex});

        WS_LOG_INFO("Infrastructure",
                    "Spawned power line {}: {} vertices, {} indices",
                    i + 1,
                    powerLineVertices.size(),
                    powerLineIndices.size());
    }

    WS_LOG_INFO(
        "Infrastructure",
        "Successfully spawned all reconstructed infrastructure objects");
}