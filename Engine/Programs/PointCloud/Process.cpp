#include "Process.hpp"
#include "../Application/World.hpp"

#include <open3d/geometry/KDTreeFlann.h>
#include <open3d/geometry/BoundingVolume.h>
#include <open3d/io/PointCloudIO.h>

#include <cmath>
#include <limits>
#include <ctime>
#include <algorithm>

using namespace worse;

// 优化的多阶段地面分割算法
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

    WS_LOG_INFO("GroundSegmentation", "Starting optimized ground segmentation on {} points", cloud->points_.size());

    // 1. 预处理 - 移除明显的离群点
    WS_LOG_INFO("GroundSegmentation", "Step 1: Preprocessing and outlier removal");
    auto preprocessed_cloud = cloud;

    // 统计离群点移除
    auto bbox           = cloud->GetAxisAlignedBoundingBox();
    auto extent         = bbox.GetExtent();
    double height_range = extent.z();

    // 如果高度范围过大，先进行粗糙的高度过滤
    if (height_range > 100.0)
    {
        std::vector<size_t> valid_indices;
        auto minBound           = bbox.GetMinBound();
        double height_threshold = minBound.z() + height_range * 0.8; // 保留底部80%的点

        for (size_t i = 0; i < cloud->points_.size(); ++i)
        {
            if (cloud->points_[i].z() < height_threshold)
            {
                valid_indices.push_back(i);
            }
        }

        if (valid_indices.size() < cloud->points_.size())
        {
            preprocessed_cloud = cloud->SelectByIndex(valid_indices);
            WS_LOG_INFO("GroundSegmentation", "Height filtering: {} -> {} points", cloud->points_.size(), preprocessed_cloud->points_.size());
        }
    }

    // 2. 自适应网格参数
    bbox          = preprocessed_cloud->GetAxisAlignedBoundingBox();
    auto minBound = bbox.GetMinBound();
    auto maxBound = bbox.GetMaxBound();
    extent        = bbox.GetExtent();

    // 根据点云密度自适应调整网格大小
    double point_density     = preprocessed_cloud->points_.size() / (extent.x() * extent.y());
    double adaptive_cellSize = cellSize;

    if (point_density < 10.0)
    {
        adaptive_cellSize = cellSize * 1.5; // 稀疏点云使用更大网格
    }
    else if (point_density > 100.0)
    {
        adaptive_cellSize = cellSize * 0.7; // 密集点云使用更小网格
    }

    WS_LOG_INFO("GroundSegmentation", "Adaptive cell size: {:.2f}m (density: {:.1f} pts/m²)", adaptive_cellSize, point_density);

    // 3. 创建多层高度网格
    int gridWidth  = static_cast<int>((maxBound.x() - minBound.x()) / adaptive_cellSize) + 1;
    int gridHeight = static_cast<int>((maxBound.y() - minBound.y()) / adaptive_cellSize) + 1;

    // 存储每个网格单元的多个高度值（最小值、中位数、标准差）
    std::vector<std::vector<std::vector<double>>> heightGridMulti(
        gridWidth,
        std::vector<std::vector<double>>(gridHeight));
    std::vector<std::vector<std::vector<size_t>>> pointGrid(
        gridWidth,
        std::vector<std::vector<size_t>>(gridHeight));

    WS_LOG_INFO("GroundSegmentation", "Step 2: Building adaptive height grid ({}x{})", gridWidth, gridHeight);

    // 4. 填充网格并收集统计信息
    for (std::size_t i = 0; i < preprocessed_cloud->points_.size(); ++i)
    {
        const auto& point = preprocessed_cloud->points_[i];
        int x             = static_cast<int>((point.x() - minBound.x()) / adaptive_cellSize);
        int y             = static_cast<int>((point.y() - minBound.y()) / adaptive_cellSize);

        if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight)
        {
            heightGridMulti[x][y].push_back(point.z());
            pointGrid[x][y].push_back(i);
        }
    }

    // 5. 计算每个网格单元的地面候选高度
    WS_LOG_INFO("GroundSegmentation", "Step 3: Computing ground candidate heights");
    std::vector<std::vector<double>> groundHeightGrid(
        gridWidth,
        std::vector<double>(gridHeight, std::numeric_limits<double>::max()));
    std::vector<std::vector<double>> heightVarianceGrid(
        gridWidth,
        std::vector<double>(gridHeight, 0.0));

    for (int x = 0; x < gridWidth; ++x)
    {
        for (int y = 0; y < gridHeight; ++y)
        {
            auto& heights = heightGridMulti[x][y];
            if (heights.empty())
                continue;

            std::sort(heights.begin(), heights.end());

            // 使用百分位数而不是最小值，更鲁棒
            size_t percentile_idx  = static_cast<size_t>(heights.size() * 0.15); // 15%分位数
            groundHeightGrid[x][y] = heights[percentile_idx];

            // 计算高度方差
            if (heights.size() > 1)
            {
                double mean = 0.0;
                for (double h : heights)
                    mean += h;
                mean /= heights.size();

                double variance = 0.0;
                for (double h : heights)
                {
                    variance += (h - mean) * (h - mean);
                }
                heightVarianceGrid[x][y] = variance / heights.size();
            }
        }
    }

    // 6. 改进的形态学滤波 - 多尺度+方向性
    WS_LOG_INFO("GroundSegmentation", "Step 4: Enhanced morphological filtering");
    std::vector<std::vector<double>> filteredHeight = groundHeightGrid;

    // 多尺度形态学开运算，增加方向性滤波
    for (double windowSize = 1.0; windowSize <= maxWindowSize; windowSize *= 1.5)
    {
        int halfWindow                            = static_cast<int>(windowSize / adaptive_cellSize / 2);
        std::vector<std::vector<double>> tempGrid = filteredHeight;

        // 改进的形态学开运算 - 考虑坡度连续性
        // 腐蚀操作 - 增加坡度约束
        for (int x = 0; x < gridWidth; ++x)
        {
            for (int y = 0; y < gridHeight; ++y)
            {
                if (filteredHeight[x][y] == std::numeric_limits<double>::max())
                    continue;

                double minHeight = filteredHeight[x][y];
                std::vector<double> neighborHeights;

                for (int dx = -halfWindow; dx <= halfWindow; ++dx)
                {
                    for (int dy = -halfWindow; dy <= halfWindow; ++dy)
                    {
                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight &&
                            filteredHeight[nx][ny] != std::numeric_limits<double>::max())
                        {
                            neighborHeights.push_back(filteredHeight[nx][ny]);
                        }
                    }
                }

                if (!neighborHeights.empty())
                {
                    // 使用鲁棒的百分位数而不是最小值
                    std::sort(neighborHeights.begin(), neighborHeights.end());
                    size_t robustIdx = static_cast<size_t>(neighborHeights.size() * 0.2);
                    tempGrid[x][y]   = neighborHeights[robustIdx];
                }
            }
        }

        // 膨胀操作 - 增加坡度连续性检查
        filteredHeight = tempGrid;
        for (int x = 0; x < gridWidth; ++x)
        {
            for (int y = 0; y < gridHeight; ++y)
            {
                if (tempGrid[x][y] == std::numeric_limits<double>::max())
                    continue;

                std::vector<double> neighborHeights;
                for (int dx = -halfWindow; dx <= halfWindow; ++dx)
                {
                    for (int dy = -halfWindow; dy <= halfWindow; ++dy)
                    {
                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight &&
                            tempGrid[nx][ny] != std::numeric_limits<double>::max())
                        {
                            neighborHeights.push_back(tempGrid[nx][ny]);
                        }
                    }
                }

                if (!neighborHeights.empty())
                {
                    std::sort(neighborHeights.begin(), neighborHeights.end());
                    size_t robustIdx     = static_cast<size_t>(neighborHeights.size() * 0.8);
                    filteredHeight[x][y] = neighborHeights[robustIdx];
                }
            }
        }
    }

    // 7. 表面平滑和插值
    WS_LOG_INFO("GroundSegmentation", "Step 5: Surface smoothing and interpolation");

    // 高斯平滑滤波
    std::vector<std::vector<double>> smoothedHeight = filteredHeight;
    int smoothKernel                                = 2; // 5x5高斯核

    for (int x = smoothKernel; x < gridWidth - smoothKernel; ++x)
    {
        for (int y = smoothKernel; y < gridHeight - smoothKernel; ++y)
        {
            if (filteredHeight[x][y] == std::numeric_limits<double>::max())
                continue;

            double weightedSum = 0.0;
            double totalWeight = 0.0;

            // 5x5 高斯权重
            std::vector<std::vector<double>> gaussianWeights = {
                {0.003, 0.013, 0.022, 0.013, 0.003},
                {0.013, 0.059, 0.097, 0.059, 0.013},
                {0.022, 0.097, 0.159, 0.097, 0.022},
                {0.013, 0.059, 0.097, 0.059, 0.013},
                {0.003, 0.013, 0.022, 0.013, 0.003}};

            for (int dx = -smoothKernel; dx <= smoothKernel; ++dx)
            {
                for (int dy = -smoothKernel; dy <= smoothKernel; ++dy)
                {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight &&
                        filteredHeight[nx][ny] != std::numeric_limits<double>::max())
                    {
                        double weight = gaussianWeights[dx + smoothKernel][dy + smoothKernel];
                        weightedSum += filteredHeight[nx][ny] * weight;
                        totalWeight += weight;
                    }
                }
            }

            if (totalWeight > 0)
            {
                smoothedHeight[x][y] = weightedSum / totalWeight;
            }
        }
    }

    filteredHeight = smoothedHeight;

    // 8. 改进的点分类 - 多准则决策
    WS_LOG_INFO("GroundSegmentation", "Step 6: Enhanced point classification");
    std::vector<size_t> groundIndices, nonGroundIndices;

    // 多准则地面点分类
    for (size_t i = 0; i < preprocessed_cloud->points_.size(); ++i)
    {
        const auto& point = preprocessed_cloud->points_[i];
        int x             = static_cast<int>((point.x() - minBound.x()) / adaptive_cellSize);
        int y             = static_cast<int>((point.y() - minBound.y()) / adaptive_cellSize);

        if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight &&
            filteredHeight[x][y] != std::numeric_limits<double>::max())
        {
            double heightDiff = point.z() - filteredHeight[x][y];

            // 准则1: 计算加权平均坡度（考虑距离衰减）
            double weightedSlope = 0.0;
            double totalWeight   = 0.0;

            for (int dx = -2; dx <= 2; ++dx)
            {
                for (int dy = -2; dy <= 2; ++dy)
                {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight &&
                        filteredHeight[nx][ny] != std::numeric_limits<double>::max())
                    {
                        double distance = std::sqrt(dx * dx + dy * dy) * adaptive_cellSize;
                        if (distance > 0 && distance < 5.0 * adaptive_cellSize) // 限制影响范围
                        {
                            double slope  = std::abs(filteredHeight[nx][ny] - filteredHeight[x][y]) / distance;
                            double weight = std::exp(-distance / (2.0 * adaptive_cellSize)); // 高斯权重
                            weightedSlope += slope * weight;
                            totalWeight += weight;
                        }
                    }
                }
            }

            if (totalWeight > 0)
            {
                weightedSlope /= totalWeight;
            }

            // 准则2: 局部高度变异性
            double localVariance = heightVarianceGrid[x][y];

            // 准则3: 动态自适应阈值
            double baseThreshold      = initialDistance;
            double slopeAdjustment    = weightedSlope * maxDistance * 0.5;
            double varianceAdjustment = std::min(localVariance * 0.1, 2.0);
            double adaptiveThreshold  = baseThreshold + slopeAdjustment + varianceAdjustment;

            // 准则4: 相对高度检查（防止悬空点被误分为地面）
            bool heightCheck   = heightDiff <= adaptiveThreshold;
            bool slopeCheck    = weightedSlope <= slopeThreshold * 1.5; // 稍微放宽坡度阈值
            bool varianceCheck = localVariance < 3.0;                   // 排除高变异性区域

            // 准则5: 邻域一致性检查
            bool neighborhoodCheck = true;
            if (heightDiff > 0.5)
            { // 对于稍高的点进行邻域检查
                int groundNeighbors = 0;
                int totalNeighbors  = 0;

                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        if (dx == 0 && dy == 0)
                            continue;

                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight)
                        {
                            auto& cellPoints = pointGrid[nx][ny];
                            for (size_t idx : cellPoints)
                            {
                                if (idx < preprocessed_cloud->points_.size())
                                {
                                    double neighborHeightDiff =
                                        preprocessed_cloud->points_[idx].z() - filteredHeight[nx][ny];
                                    if (neighborHeightDiff <= adaptiveThreshold)
                                    {
                                        groundNeighbors++;
                                    }
                                    totalNeighbors++;
                                }
                            }
                        }
                    }
                }

                if (totalNeighbors > 0)
                {
                    double groundRatio = static_cast<double>(groundNeighbors) / totalNeighbors;
                    neighborhoodCheck  = groundRatio > 0.3; // 至少30%的邻居是地面点
                }
            }

            // 综合决策
            if (heightCheck && slopeCheck && varianceCheck && neighborhoodCheck)
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

    // 9. 后处理 - 地面点连通性分析和噪声去除
    WS_LOG_INFO("GroundSegmentation", "Step 7: Post-processing and connectivity analysis");

    // 如果检测到的地面点过少，放宽条件
    double groundRatio = static_cast<double>(groundIndices.size()) / preprocessed_cloud->points_.size();
    if (groundRatio < 0.1)
    { // 地面点少于10%
        WS_LOG_WARN("GroundSegmentation", "Ground ratio too low ({:.1f}%), relaxing criteria", groundRatio * 100);

        groundIndices.clear();
        nonGroundIndices.clear();

        // 使用更宽松的条件重新分类
        for (size_t i = 0; i < preprocessed_cloud->points_.size(); ++i)
        {
            const auto& point = preprocessed_cloud->points_[i];
            int x             = static_cast<int>((point.x() - minBound.x()) / adaptive_cellSize);
            int y             = static_cast<int>((point.y() - minBound.y()) / adaptive_cellSize);

            if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight &&
                filteredHeight[x][y] != std::numeric_limits<double>::max())
            {
                double heightDiff = point.z() - filteredHeight[x][y];
                if (heightDiff <= maxDistance * 2.0)
                { // 使用更大的阈值
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
    }

    // 10. 创建结果点云
    WS_LOG_INFO("GroundSegmentation", "Step 8: Creating result point clouds");
    result.groundPoints    = preprocessed_cloud->SelectByIndex(groundIndices);
    result.nonGroundPoints = preprocessed_cloud->SelectByIndex(nonGroundIndices);

    // 构造改进的地面平面模型（使用RANSAC平面拟合）
    if (!groundIndices.empty() && result.groundPoints->points_.size() > 3)
    {
        // 使用RANSAC进行平面拟合
        auto [plane_model, inliers] = result.groundPoints->SegmentPlane(
            0.5,
            3,
            1000); // distance_threshold, ransac_n, num_iterations

        if (inliers.size() > groundIndices.size() * 0.3)
        { // 至少30%的内点
            result.groundPlaneModel = plane_model;
        }
        else
        {
            // 回退到简单的水平面
            double avgZ = 0.0;
            for (size_t idx : groundIndices)
            {
                avgZ += preprocessed_cloud->points_[idx].z();
            }
            avgZ /= groundIndices.size();
            result.groundPlaneModel = Eigen::Vector4d(0, 0, 1, -avgZ);
        }
    }

    double finalGroundRatio = static_cast<double>(groundIndices.size()) / preprocessed_cloud->points_.size();
    WS_LOG_INFO("GroundSegmentation",
                "Segmentation completed: {:.1f}% ground points ({}/{})",
                finalGroundRatio * 100,
                groundIndices.size(),
                preprocessed_cloud->points_.size());

    return result;
}

// 针对电力基础设施优化的聚类分析
ClusteringResult
performClustering(std::shared_ptr<open3d::geometry::PointCloud> cloud,
                  double eps, int min_points)
{
    ClusteringResult result;

    if (cloud->points_.empty())
    {
        return result;
    }

    WS_LOG_INFO("PowerInfrastructure",
                "Starting power infrastructure clustering on {} points...",
                cloud->points_.size());

    // 1. 高度分层预处理 - 专门针对电力设施
    auto bbox           = cloud->GetAxisAlignedBoundingBox();
    double min_z        = bbox.GetMinBound().z();
    double max_z        = bbox.GetMaxBound().z();
    double height_range = max_z - min_z;

    WS_LOG_INFO("PowerInfrastructure",
                "Point cloud height range: {:.2f}m (from {:.2f}m to {:.2f}m)",
                height_range,
                min_z,
                max_z);

    // 2. 多层次聚类策略
    std::vector<std::shared_ptr<open3d::geometry::PointCloud>> all_clusters;

    // 2.1 高空区域聚类（电力塔顶部和高压线）
    std::vector<size_t> high_indices;
    double high_threshold = min_z + height_range * 0.7; // 上层30%

    for (size_t i = 0; i < cloud->points_.size(); ++i)
    {
        if (cloud->points_[i].z() > high_threshold)
        {
            high_indices.push_back(i);
        }
    }

    if (!high_indices.empty())
    {
        auto high_cloud = cloud->SelectByIndex(high_indices);
        WS_LOG_INFO("PowerInfrastructure", "High-altitude clustering: {} points", high_cloud->points_.size());

        // === 使用自适应参数优化聚类 ===
        auto adaptiveParams = calculateOptimalClusteringParams(high_cloud);

        // 对高空区域使用更保守的参数避免断线
        // 使用较大的eps确保电力线连通性，避免过度分割
        double high_eps     = std::max(adaptiveParams.eps * 1.2, 2.0);    // 确保足够大的连接距离
        int high_min_points = std::max(2, adaptiveParams.min_points / 3); // 更小的min_points保持细线

        WS_LOG_INFO("PowerInfrastructure",
                    "High-altitude conservative clustering: eps={:.2f}, min_points={} (adaptive base: eps={:.2f})",
                    high_eps,
                    high_min_points,
                    adaptiveParams.eps);

        auto high_labels = high_cloud->ClusterDBSCAN(high_eps, high_min_points);

        // === 收集电力线候选 ===
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>> high_power_line_candidates;
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>> high_other_clusters;

        int max_high_label = -1;
        if (!high_labels.empty())
        {
            max_high_label = *std::max_element(high_labels.begin(), high_labels.end());
        }

        for (int i = 0; i <= max_high_label; ++i)
        {
            std::vector<size_t> cluster_indices;
            for (size_t j = 0; j < high_labels.size(); ++j)
            {
                if (high_labels[j] == i)
                {
                    cluster_indices.push_back(j);
                }
            }

            if (!cluster_indices.empty() && cluster_indices.size() >= 3)
            {
                auto cluster = high_cloud->SelectByIndex(cluster_indices);

                // === 改进的电力线判定条件 ===
                auto cluster_bbox = cluster->GetAxisAlignedBoundingBox();
                auto extent       = cluster_bbox.GetExtent();

                double length    = std::max({extent.x(), extent.y()});
                double width     = std::min({extent.x(), extent.y()});
                double height    = extent.z();
                double linearity = (width > 0.01) ? length / width : length / 0.01;

                // 更宽松的电力线判定：降低长度要求，因为断线可能导致短段
                // 也考虑可能的短电力线段和断线
                bool is_power_line = ((linearity > 2.5 && length > 3.0 && height < 12.0) ||             // 主要条件：相对线性且不太高
                                      (linearity > 5.0 && length > 1.5) ||                              // 高线性度的短段
                                      (cluster->points_.size() > 15 && linearity > 1.8 && length > 2.0) // 足够点的中等线性段
                );

                if (is_power_line)
                {
                    high_power_line_candidates.push_back(cluster);
                    WS_LOG_INFO("PowerInfrastructure",
                                "Power line candidate {}: {} points, length={:.1f}m, linearity={:.1f}, height_span={:.1f}m",
                                i,
                                cluster->points_.size(),
                                length,
                                linearity,
                                height);
                }
                else
                {
                    high_other_clusters.push_back(cluster);
                    WS_LOG_INFO("PowerInfrastructure",
                                "High structure {}: {} points, size={:.1f}x{:.1f}x{:.1f}m, linearity={:.1f}",
                                i,
                                cluster->points_.size(),
                                extent.x(),
                                extent.y(),
                                extent.z(),
                                linearity);
                }
            }
        }

        // === 断线合并处理 ===
        if (!high_power_line_candidates.empty())
        {
            WS_LOG_INFO("PowerInfrastructure",
                        "Attempting to merge {} power line segments",
                        high_power_line_candidates.size());

            auto [merged_lines, mergeStats] = mergeBrokenPowerLines(high_power_line_candidates,
                                                                    25.0,  // maxGapDistance - 更大的断线距离适应实际情况
                                                                    8.0,   // maxHeightDiff - 考虑悬链线高度变化
                                                                    35.0); // maxAngleDiff - 更宽松的角度容差

            // 存储合并统计信息到静态变量，供后续参数生成使用
            static PowerLineMergeStats globalMergeStats;
            globalMergeStats = mergeStats;

            // 添加合并后的电力线
            for (auto& merged_line : merged_lines)
            {
                all_clusters.push_back(merged_line);
            }

            WS_LOG_INFO("PowerInfrastructure",
                        "Power line merging: {} segments -> {} lines (quality: {:.2f})",
                        high_power_line_candidates.size(),
                        merged_lines.size(),
                        mergeStats.averageMergeQuality);
        }

        // 添加其他高空结构
        for (auto& other_cluster : high_other_clusters)
        {
            all_clusters.push_back(other_cluster);
        }

        WS_LOG_INFO("PowerInfrastructure", "High-altitude processing completed");
    }

    // 2.2 中空区域聚类（电力塔主体和中压线）
    std::vector<size_t> mid_indices;
    double mid_low  = min_z + height_range * 0.2; // 下层20%以上
    double mid_high = min_z + height_range * 0.7; // 上层30%以下

    for (size_t i = 0; i < cloud->points_.size(); ++i)
    {
        double z = cloud->points_[i].z();
        if (z > mid_low && z <= mid_high)
        {
            mid_indices.push_back(i);
        }
    }

    if (!mid_indices.empty())
    {
        auto mid_cloud = cloud->SelectByIndex(mid_indices);
        WS_LOG_INFO("PowerInfrastructure", "Mid-altitude clustering: {} points", mid_cloud->points_.size());

        // 中空区域使用标准参数
        auto mid_labels = mid_cloud->ClusterDBSCAN(eps, min_points);

        int max_mid_label = -1;
        if (!mid_labels.empty())
        {
            max_mid_label = *std::max_element(mid_labels.begin(), mid_labels.end());
        }

        for (int i = 0; i <= max_mid_label; ++i)
        {
            std::vector<size_t> cluster_indices;
            for (size_t j = 0; j < mid_labels.size(); ++j)
            {
                if (mid_labels[j] == i)
                {
                    cluster_indices.push_back(j);
                }
            }

            if (!cluster_indices.empty() && cluster_indices.size() >= min_points)
            {
                auto cluster = mid_cloud->SelectByIndex(cluster_indices);
                all_clusters.push_back(cluster);
            }
        }

        WS_LOG_INFO("PowerInfrastructure", "Mid-altitude clusters: {}", max_mid_label + 1);
    }

    // 2.3 低空区域聚类（电力塔基座和低矮设施）
    std::vector<size_t> low_indices;
    double low_threshold = min_z + height_range * 0.2; // 下层20%

    for (size_t i = 0; i < cloud->points_.size(); ++i)
    {
        if (cloud->points_[i].z() <= low_threshold)
        {
            low_indices.push_back(i);
        }
    }

    if (!low_indices.empty())
    {
        auto low_cloud = cloud->SelectByIndex(low_indices);
        WS_LOG_INFO("PowerInfrastructure", "Low-altitude clustering: {} points", low_cloud->points_.size());

        // 低空区域使用更大的eps以合并基座结构
        double low_eps     = eps * 1.5;
        int low_min_points = min_points * 2;

        auto low_labels = low_cloud->ClusterDBSCAN(low_eps, low_min_points);

        int max_low_label = -1;
        if (!low_labels.empty())
        {
            max_low_label = *std::max_element(low_labels.begin(), low_labels.end());
        }

        for (int i = 0; i <= max_low_label; ++i)
        {
            std::vector<size_t> cluster_indices;
            for (size_t j = 0; j < low_labels.size(); ++j)
            {
                if (low_labels[j] == i)
                {
                    cluster_indices.push_back(j);
                }
            }

            if (!cluster_indices.empty() && cluster_indices.size() >= low_min_points)
            {
                auto cluster = low_cloud->SelectByIndex(cluster_indices);
                all_clusters.push_back(cluster);
            }
        }

        WS_LOG_INFO("PowerInfrastructure", "Low-altitude clusters: {}", max_low_label + 1);
    }

    // 3. 形状和几何特征过滤
    WS_LOG_INFO("PowerInfrastructure", "Filtering clusters by shape characteristics...");

    for (const auto& cluster : all_clusters)
    {
        if (!cluster || cluster->points_.empty())
            continue;

        auto cluster_bbox = cluster->GetAxisAlignedBoundingBox();
        auto extent       = cluster_bbox.GetExtent();
        auto center       = cluster_bbox.GetCenter();

        double width           = extent.x();
        double depth           = extent.y();
        double height          = extent.z();
        double volume          = width * depth * height;
        double relative_height = center.z() - min_z;

        // 计算特征比率
        double aspect_ratio_xy = std::max(width, depth) / std::min(width, depth);
        double aspect_ratio_z  = height / std::min(width, depth);
        double point_density   = cluster->points_.size() / std::max(volume, 0.1);

        // 更宽松的过滤条件，专门针对电力设施
        bool size_ok   = (width > 0.5 && depth > 0.5 && height > 1.0); // 最小尺寸要求
        bool height_ok = relative_height > 5.0;                        // 相对地面5米以上
        bool points_ok = cluster->points_.size() >= 10;                // 最少10个点

        // 特殊形状检测
        bool tower_like     = (aspect_ratio_z > 3.0 && aspect_ratio_xy < 4.0 && height > 15.0);
        bool line_like      = (aspect_ratio_xy > 8.0 || (width > 20.0 || depth > 20.0));
        bool structure_like = (volume > 20.0 && point_density > 0.05);

        if (size_ok && height_ok && points_ok && (tower_like || line_like || structure_like))
        {
            result.clusters.push_back(cluster);

            WS_LOG_INFO("PowerInfrastructure",
                        "Valid cluster: {} points, {:.1f}x{:.1f}x{:.1f}m, aspect_xy={:.1f}, aspect_z={:.1f}, type={}",
                        cluster->points_.size(),
                        width,
                        depth,
                        height,
                        aspect_ratio_xy,
                        aspect_ratio_z,
                        tower_like ? "tower-like" : (line_like ? "line-like" : "structure-like"));
        }
    }

    WS_LOG_INFO("PowerInfrastructure",
                "Clustering completed: {} valid clusters found from {} total clusters",
                result.clusters.size(),
                all_clusters.size());

    return result;
}

// 专业的电力基础设施分类算法
InfrastructureClassification classifyInfrastructure(
    const std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& clusters,
    std::shared_ptr<open3d::geometry::PointCloud> ground_points)
{
    InfrastructureClassification result;
    result.groundPoints = ground_points;

    if (clusters.empty())
    {
        WS_LOG_WARN("PowerInfrastructure", "No clusters provided for classification");
        return result;
    }

    // 计算地面基准高度
    double ground_z = 0.0;
    if (ground_points && !ground_points->points_.empty())
    {
        auto ground_bbox = ground_points->GetAxisAlignedBoundingBox();
        ground_z         = ground_bbox.GetMaxBound().z();
    }

    WS_LOG_INFO("PowerInfrastructure", "Classifying {} clusters with ground level at {:.2f}m", clusters.size(), ground_z);

    // 分类候选者存储
    struct TowerCandidate
    {
        std::shared_ptr<open3d::geometry::PointCloud> cloud;
        double score;
        double height;
        double volume;
        math::Vector3 position;
    };

    struct PowerLineCandidate
    {
        std::shared_ptr<open3d::geometry::PointCloud> cloud;
        double score;
        double length;
        double height;
        math::Vector3 start, end;
    };

    std::vector<TowerCandidate> tower_candidates;
    std::vector<PowerLineCandidate> powerline_candidates;

    // 遍历所有聚类进行分类
    for (size_t idx = 0; idx < clusters.size(); ++idx)
    {
        const auto& cluster = clusters[idx];
        if (!cluster || cluster->points_.empty())
            continue;

        auto bbox      = cluster->GetAxisAlignedBoundingBox();
        auto extent    = bbox.GetExtent();
        auto center    = bbox.GetCenter();
        auto min_bound = bbox.GetMinBound();
        auto max_bound = bbox.GetMaxBound();

        double width           = extent.x();
        double depth           = extent.y();
        double height          = extent.z();
        double relative_height = center.z() - ground_z;
        double volume          = width * depth * height;
        double base_area       = width * depth;
        size_t point_count     = cluster->points_.size();

        // 计算几何特征
        double aspect_ratio_xy = std::max(width, depth) / std::min(width, depth);
        double aspect_ratio_z  = height / std::min(width, depth);
        double compactness     = point_count / std::max(volume, 0.1);
        double slenderness     = height / std::sqrt(base_area);

        WS_LOG_DEBUG("PowerInfrastructure",
                     "Cluster {}: {:.1f}x{:.1f}x{:.1f}m, {} points, height={:.1f}m, aspect_xy={:.1f}, aspect_z={:.1f}",
                     idx,
                     width,
                     depth,
                     height,
                     point_count,
                     relative_height,
                     aspect_ratio_xy,
                     aspect_ratio_z);

        // === 电力塔识别算法 ===
        // 更精准的电力塔识别标准
        bool tower_height_ok          = relative_height > 20.0;                          // 相对地面20米以上
        bool tower_absolute_height_ok = height > 25.0;                                   // 绝对高度25米以上
        bool tower_shape_ok           = (aspect_ratio_z > 2.5 && aspect_ratio_z < 15.0); // 高而不过分细长
        bool tower_base_ok            = (aspect_ratio_xy < 5.0 && base_area > 15.0);     // 底面相对方正且足够大
        bool tower_volume_ok          = volume > 200.0;                                  // 足够的体积
        bool tower_density_ok         = compactness > 0.05;                              // 合理的点密度
        bool tower_points_ok          = point_count > 80;                                // 足够的点数

        // 高级形状检测 - 检查是否有塔状结构特征
        bool tower_profile_ok = false;
        if (tower_height_ok && tower_shape_ok)
        {
            // 分析垂直剖面 - 电力塔通常底部宽，顶部窄
            int layers          = 5;
            double layer_height = height / layers;
            std::vector<double> layer_areas;

            for (int layer = 0; layer < layers; ++layer)
            {
                double z_min = min_bound.z() + layer * layer_height;
                double z_max = z_min + layer_height;

                std::vector<size_t> layer_indices;
                for (size_t i = 0; i < cluster->points_.size(); ++i)
                {
                    double z = cluster->points_[i].z();
                    if (z >= z_min && z < z_max)
                    {
                        layer_indices.push_back(i);
                    }
                }

                if (!layer_indices.empty())
                {
                    auto layer_cloud  = cluster->SelectByIndex(layer_indices);
                    auto layer_bbox   = layer_cloud->GetAxisAlignedBoundingBox();
                    auto layer_extent = layer_bbox.GetExtent();
                    double area       = layer_extent.x() * layer_extent.y();
                    layer_areas.push_back(area);
                }
            }

            // 检查是否有锥形特征（底部面积大于顶部）
            if (layer_areas.size() >= 3)
            {
                double bottom_area = layer_areas[0];
                double top_area    = layer_areas.back();
                tower_profile_ok   = (bottom_area > top_area * 1.2); // 底部比顶部大20%以上
            }
        }

        if (tower_height_ok && tower_absolute_height_ok && tower_shape_ok &&
            tower_base_ok && tower_volume_ok && tower_density_ok && tower_points_ok)
        {

            // 计算电力塔评分
            double tower_score = 0.0;
            tower_score += relative_height * 0.3; // 高度权重
            tower_score += volume * 0.001;        // 体积权重
            tower_score += compactness * 20.0;    // 密度权重
            tower_score += slenderness * 2.0;     // 细长度权重

            if (tower_profile_ok)
                tower_score += 50.0; // 锥形特征加分

            TowerCandidate candidate;
            candidate.cloud    = cluster;
            candidate.score    = tower_score;
            candidate.height   = relative_height;
            candidate.volume   = volume;
            candidate.position = math::Vector3(
                static_cast<float>(center.x()),
                static_cast<float>(center.y()),
                static_cast<float>(center.z()));

            tower_candidates.push_back(candidate);

            WS_LOG_INFO("PowerInfrastructure",
                        "Tower candidate {}: score={:.1f}, height={:.1f}m, volume={:.1f}m³, profile={}",
                        idx,
                        tower_score,
                        relative_height,
                        volume,
                        tower_profile_ok ? "yes" : "no");
        }

        // === 输电线识别算法 ===
        // 更精准的输电线识别标准
        bool line_height_ok    = relative_height > 8.0 && relative_height < 80.0; // 8-80米高度范围
        bool line_length_ok    = (std::max(width, depth) > 30.0);                 // 至少30米长
        bool line_shape_ok     = aspect_ratio_xy > 6.0;                           // 明显的线状结构
        bool line_thickness_ok = (height < 8.0 || std::min(width, depth) < 3.0);  // 相对较薄
        bool line_volume_ok    = volume > 10.0 && volume < 800.0;                 // 体积范围控制
        bool line_points_ok    = point_count > 30 && point_count < 5000;          // 点数范围

        // 线性度检测 - 检查点的分布是否接近直线
        bool line_linearity_ok = false;
        if (line_length_ok && line_shape_ok)
        {
            // 简化的线性度检测 - 检查点云的最长轴与总体积的比例
            double max_dimension   = std::max({width, depth, height});
            double min_dimension   = std::min({width, depth, height});
            double dimension_ratio = max_dimension / min_dimension;

            // 如果最长维度是其他维度的8倍以上，认为是线性的
            line_linearity_ok = dimension_ratio > 8.0;

            // 额外检查：点云在主方向上的分布密度
            if (!line_linearity_ok && aspect_ratio_xy > 10.0)
            {
                double main_axis_length = std::max(width, depth);
                double cross_section    = volume / main_axis_length;
                double thickness        = std::sqrt(cross_section);

                // 如果厚度相对于长度很小，也认为是线性的
                line_linearity_ok = (thickness / main_axis_length) < 0.1;
            }
        }

        if (line_height_ok && line_length_ok && line_shape_ok &&
            line_thickness_ok && line_volume_ok && line_points_ok)
        {

            // 计算输电线评分
            double line_score = 0.0;
            line_score += std::max(width, depth) * 0.5;    // 长度权重
            line_score += aspect_ratio_xy * 2.0;           // 长宽比权重
            line_score += (100.0 - relative_height) * 0.3; // 适中高度加分

            if (line_linearity_ok)
                line_score += 30.0; // 线性度加分

            PowerLineCandidate candidate;
            candidate.cloud  = cluster;
            candidate.score  = line_score;
            candidate.length = std::max(width, depth);
            candidate.height = relative_height;

            // 确定起点和终点
            if (width > depth)
            {
                candidate.start = math::Vector3(static_cast<float>(min_bound.x()),
                                                static_cast<float>(center.y()),
                                                static_cast<float>(center.z()));
                candidate.end   = math::Vector3(static_cast<float>(max_bound.x()),
                                              static_cast<float>(center.y()),
                                              static_cast<float>(center.z()));
            }
            else
            {
                candidate.start = math::Vector3(static_cast<float>(center.x()),
                                                static_cast<float>(min_bound.y()),
                                                static_cast<float>(center.z()));
                candidate.end   = math::Vector3(static_cast<float>(center.x()),
                                              static_cast<float>(max_bound.y()),
                                              static_cast<float>(center.z()));
            }

            powerline_candidates.push_back(candidate);

            WS_LOG_INFO("PowerInfrastructure",
                        "PowerLine candidate {}: score={:.1f}, length={:.1f}m, height={:.1f}m, linearity={}",
                        idx,
                        line_score,
                        candidate.length,
                        relative_height,
                        line_linearity_ok ? "yes" : "no");
        }
    }

    // 选择最佳电力塔
    if (!tower_candidates.empty())
    {
        std::sort(tower_candidates.begin(), tower_candidates.end(), [](const TowerCandidate& a, const TowerCandidate& b)
                  {
                      return a.score > b.score;
                  });

        result.towerPoints = tower_candidates[0].cloud;

        auto tower_bbox = result.towerPoints->GetAxisAlignedBoundingBox();
        auto min_bound  = tower_bbox.GetMinBound();
        auto max_bound  = tower_bbox.GetMaxBound();
        result.towerMin = math::Vector3(static_cast<float>(min_bound.x()),
                                        static_cast<float>(min_bound.y()),
                                        static_cast<float>(min_bound.z()));
        result.towerMax = math::Vector3(static_cast<float>(max_bound.x()),
                                        static_cast<float>(max_bound.y()),
                                        static_cast<float>(max_bound.z()));

        WS_LOG_INFO("PowerInfrastructure",
                    "Selected best tower: score={:.1f}, height={:.1f}m, volume={:.1f}m³",
                    tower_candidates[0].score,
                    tower_candidates[0].height,
                    tower_candidates[0].volume);
    }

    // 选择输电线（可以有多条）
    if (!powerline_candidates.empty())
    {
        // 按评分排序
        std::sort(powerline_candidates.begin(), powerline_candidates.end(), [](const PowerLineCandidate& a, const PowerLineCandidate& b)
                  {
                      return a.score > b.score;
                  });

        // 选择前几条最佳的输电线
        size_t max_lines = std::min(static_cast<size_t>(8), powerline_candidates.size());
        for (size_t i = 0; i < max_lines; ++i)
        {
            result.powerLines.push_back(powerline_candidates[i].cloud);

            WS_LOG_INFO("PowerInfrastructure",
                        "Selected powerline {}: score={:.1f}, length={:.1f}m, height={:.1f}m",
                        i + 1,
                        powerline_candidates[i].score,
                        powerline_candidates[i].length,
                        powerline_candidates[i].height);
        }
    }

    // 距离过滤 - 如果有电力塔，优先选择附近的输电线
    if (result.towerPoints && !result.powerLines.empty())
    {
        auto tower_center = result.towerPoints->GetAxisAlignedBoundingBox().GetCenter();

        std::vector<std::shared_ptr<open3d::geometry::PointCloud>> filtered_lines;
        for (const auto& line : result.powerLines)
        {
            auto line_center = line->GetAxisAlignedBoundingBox().GetCenter();
            double distance  = std::sqrt(
                std::pow(line_center.x() - tower_center.x(), 2) +
                std::pow(line_center.y() - tower_center.y(), 2));

            // 保留距离电力塔500米以内的输电线
            if (distance < 500.0)
            {
                filtered_lines.push_back(line);
            }
        }

        if (!filtered_lines.empty())
        {
            result.powerLines = filtered_lines;
            WS_LOG_INFO("PowerInfrastructure",
                        "Filtered powerlines by tower proximity: {} lines within 500m",
                        filtered_lines.size());
        }
    }

    WS_LOG_INFO("PowerInfrastructure",
                "Classification completed: {} towers, {} power lines",
                result.towerPoints ? 1 : 0,
                result.powerLines.size());

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

// === 改进的电力线分析函数 ===

// 断线检测和合并函数 - 改进版本，专门处理电力线碎片化问题
std::pair<std::vector<std::shared_ptr<open3d::geometry::PointCloud>>, PowerLineMergeStats>
mergeBrokenPowerLines(const std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& powerLineCandidates,
                      double maxGapDistance, double maxHeightDiff, double maxAngleDiff)
{
    if (powerLineCandidates.size() < 2)
    {
        PowerLineMergeStats stats;
        stats.originalSegmentCount = powerLineCandidates.size();
        stats.mergedLineCount      = powerLineCandidates.size();
        stats.averageMergeQuality  = 1.0;
        stats.processingNote       = "无需合并";
        for (size_t i = 0; i < powerLineCandidates.size(); ++i)
        {
            stats.segmentCounts.push_back(1);
        }
        return {powerLineCandidates, stats};
    }

    WS_LOG_INFO("PowerLineMerging", "Starting advanced broken line merging for {} candidates", powerLineCandidates.size());

    // 计算每条线的详细几何特征
    struct AdvancedLineFeature
    {
        std::shared_ptr<open3d::geometry::PointCloud> cloud;
        worse::math::Vector3 start, end, center, direction;
        double length, height, confidence;
        std::array<worse::math::Vector3, 2> endpoints;
        int originalIndex;
        bool isProcessed;
    };

    std::vector<AdvancedLineFeature> features;

    for (size_t i = 0; i < powerLineCandidates.size(); ++i)
    {
        auto cloud = powerLineCandidates[i];
        if (!cloud || cloud->points_.empty())
            continue;

        AdvancedLineFeature feature;
        feature.cloud         = cloud;
        feature.originalIndex = i;
        feature.isProcessed   = false;

        // === 精确端点检测 ===
        auto bbox     = cloud->GetAxisAlignedBoundingBox();
        auto minBound = bbox.GetMinBound();
        auto maxBound = bbox.GetMaxBound();
        auto extent   = bbox.GetExtent();
        auto centerPt = bbox.GetCenter();

        feature.center = worse::math::Vector3(centerPt.x(), centerPt.y(), centerPt.z());
        feature.height = centerPt.z();
        feature.length = std::max(extent.x(), extent.y());

        // === 主成分分析确定真实的线路方向 ===
        worse::math::Vector3 meanPoint = feature.center;

        // 计算协方差矩阵
        double cov_xx = 0, cov_yy = 0, cov_xy = 0;
        for (const auto& point : cloud->points_)
        {
            double dx = point.x() - meanPoint.x;
            double dy = point.y() - meanPoint.y;
            cov_xx += dx * dx;
            cov_yy += dy * dy;
            cov_xy += dx * dy;
        }
        cov_xx /= cloud->points_.size();
        cov_yy /= cloud->points_.size();
        cov_xy /= cloud->points_.size();

        // 计算主方向特征向量
        double trace   = cov_xx + cov_yy;
        double det     = cov_xx * cov_yy - cov_xy * cov_xy;
        double lambda1 = (trace + std::sqrt(trace * trace - 4 * det)) / 2;

        double eigen_x = 1.0, eigen_y = 0.0;
        if (std::abs(cov_xy) > 1e-6)
        {
            eigen_y     = (lambda1 - cov_xx) / cov_xy;
            double norm = std::sqrt(eigen_x * eigen_x + eigen_y * eigen_y);
            eigen_x /= norm;
            eigen_y /= norm;
        }
        else if (cov_xx > cov_yy)
        {
            eigen_x = 1.0;
            eigen_y = 0.0;
        }
        else
        {
            eigen_x = 0.0;
            eigen_y = 1.0;
        }

        feature.direction = worse::math::Vector3(eigen_x, eigen_y, 0.0);

        // === 计算真实端点（沿主方向投影最远的两个点） ===
        double min_proj = std::numeric_limits<double>::max();
        double max_proj = std::numeric_limits<double>::lowest();
        worse::math::Vector3 min_point, max_point;

        for (const auto& point : cloud->points_)
        {
            worse::math::Vector3 p(point.x(), point.y(), point.z());
            double projection = math::dot(p - feature.center, feature.direction);

            if (projection < min_proj)
            {
                min_proj  = projection;
                min_point = p;
            }
            if (projection > max_proj)
            {
                max_proj  = projection;
                max_point = p;
            }
        }

        feature.start        = min_point;
        feature.end          = max_point;
        feature.endpoints[0] = min_point;
        feature.endpoints[1] = max_point;

        // === 置信度评估（线性度和点密度） ===
        double linearity   = (extent.y() > 0.01) ? std::max(extent.x(), extent.y()) / std::min(extent.x(), extent.y()) : 100.0;
        double density     = cloud->points_.size() / std::max(feature.length, 0.1);
        feature.confidence = std::min(1.0, (linearity / 10.0) * (density / 5.0));

        features.push_back(feature);
    }

    // === 智能合并算法 ===
    std::vector<std::vector<int>> mergeGroups;

    for (size_t i = 0; i < features.size(); ++i)
    {
        if (features[i].isProcessed)
            continue;

        std::vector<int> currentGroup;
        std::queue<int> processQueue;

        processQueue.push(i);
        features[i].isProcessed = true;

        // BFS搜索所有连通的线段
        while (!processQueue.empty())
        {
            int current = processQueue.front();
            processQueue.pop();
            currentGroup.push_back(current);

            const auto& currentLine = features[current];

            // 搜索可连接的线段
            for (size_t j = 0; j < features.size(); ++j)
            {
                if (features[j].isProcessed)
                    continue;

                const auto& candidateLine = features[j];

                // === 计算连接可能性 ===
                std::array<double, 4> distances = {
                    math::distance(currentLine.endpoints[0], candidateLine.endpoints[0]),
                    math::distance(currentLine.endpoints[0], candidateLine.endpoints[1]),
                    math::distance(currentLine.endpoints[1], candidateLine.endpoints[0]),
                    math::distance(currentLine.endpoints[1], candidateLine.endpoints[1])};

                double minDistance = *std::min_element(distances.begin(), distances.end());
                double heightDiff  = std::abs(currentLine.height - candidateLine.height);

                // 方向一致性检查
                double directionDot        = std::abs(math::dot(currentLine.direction, candidateLine.direction));
                double directionSimilarity = directionDot; // 0-1, 1表示完全平行

                // === 改进的合并条件 ===
                // 1. 距离条件：考虑线段长度的自适应阈值
                double adaptiveGapThreshold = maxGapDistance * (1.0 + std::max(currentLine.length, candidateLine.length) / 50.0);

                // 2. 高度条件：考虑悬链线的自然下垂
                double adaptiveHeightThreshold = maxHeightDiff * (1.0 + minDistance / 20.0);

                // 3. 方向条件：必须相对平行
                bool isConnectable = (minDistance <= adaptiveGapThreshold &&
                                      heightDiff <= adaptiveHeightThreshold &&
                                      directionSimilarity > 0.7); // cos(45°) ≈ 0.707

                if (isConnectable)
                {
                    features[j].isProcessed = true;
                    processQueue.push(j);

                    WS_LOG_INFO("PowerLineMerging",
                                "Connecting line {} to group (distance={:.1f}m, height_diff={:.1f}m, direction_sim={:.2f})",
                                j,
                                minDistance,
                                heightDiff,
                                directionSimilarity);
                }
            }
        }

        mergeGroups.push_back(currentGroup);
    }

    // === 生成优化的合并点云 ===
    std::vector<std::shared_ptr<open3d::geometry::PointCloud>> mergedLines;

    for (const auto& group : mergeGroups)
    {
        if (group.size() == 1)
        {
            // 单独的线段，直接添加
            mergedLines.push_back(features[group[0]].cloud);
        }
        else
        {
            // 合并多个线段并进行优化
            auto mergedCloud = std::make_shared<open3d::geometry::PointCloud>();

            for (int idx : group)
            {
                *mergedCloud += *features[idx].cloud;
            }

            // === 合并后优化 ===
            // 1. 去重
            mergedCloud->RemoveDuplicatedPoints();

            // 2. 如果点太少，保持原样；否则进行轻微平滑
            if (mergedCloud->points_.size() > 20)
            {
                // 轻微的统计离群点清理，保持线的连续性
                try
                {
                    auto [cleaned, indices] = mergedCloud->RemoveStatisticalOutliers(8, 2.5);
                    if (cleaned->points_.size() > mergedCloud->points_.size() * 0.8) // 保留至少80%的点
                    {
                        mergedCloud = cleaned;
                    }
                }
                catch (...)
                {
                    // 如果平滑失败，保持原点云
                }
            }

            mergedLines.push_back(mergedCloud);

            WS_LOG_INFO("PowerLineMerging",
                        "Created optimized merged line from {} segments with {} points",
                        group.size(),
                        mergedCloud->points_.size());
        }
    }

    // === 生成合并统计信息 ===
    PowerLineMergeStats stats;
    stats.originalSegmentCount = powerLineCandidates.size();
    stats.mergedLineCount      = mergedLines.size();

    // 计算每条合并线包含的原始段数
    double totalQuality = 0.0;
    for (const auto& group : mergeGroups)
    {
        stats.segmentCounts.push_back(group.size());

        // 简单的质量评估：基于合并段数
        double groupQuality = (group.size() > 1) ? 0.9 - (group.size() - 2) * 0.1 : 1.0;
        totalQuality += std::max(0.3, groupQuality);
    }

    stats.averageMergeQuality = (mergeGroups.empty()) ? 1.0 : totalQuality / mergeGroups.size();

    if (stats.originalSegmentCount > stats.mergedLineCount)
    {
        stats.processingNote = "成功合并断开的电力线段";
    }
    else
    {
        stats.processingNote = "电力线完整，无需合并";
    }

    WS_LOG_INFO("PowerLineMerging",
                "Advanced merging completed: {} original -> {} merged lines, avg quality: {:.2f}",
                powerLineCandidates.size(),
                mergedLines.size(),
                stats.averageMergeQuality);

    return {mergedLines, stats};
}

// 改进的聚类参数计算
AdaptiveClusteringParams calculateOptimalClusteringParams(std::shared_ptr<open3d::geometry::PointCloud> cloud)
{
    AdaptiveClusteringParams params;

    if (!cloud || cloud->points_.empty())
    {
        params.eps               = 2.0; // 更大的默认eps防止断线
        params.min_points        = 3;   // 更小的默认min_points
        params.density_threshold = 0.01;
        return params;
    }

    // 计算点云几何特征
    auto bbox   = cloud->GetAxisAlignedBoundingBox();
    auto extent = bbox.GetExtent();

    // === 电力线特征分析 ===
    double max_length = std::max({extent.x(), extent.y()});
    double min_width  = std::min({extent.x(), extent.y()});
    double height     = extent.z();
    double linearity  = (min_width > 0.01) ? max_length / min_width : max_length / 0.01;

    WS_LOG_INFO("PowerLineAnalysis",
                "Cloud geometry: length={:.1f}m, width={:.1f}m, height={:.1f}m, linearity={:.1f}",
                max_length,
                min_width,
                height,
                linearity);

    // === 距离分析专门针对线性结构 ===
    std::vector<double> nearest_distances;
    nearest_distances.reserve(std::min(static_cast<size_t>(1000), cloud->points_.size()));

    // 采样最近邻距离
    int sample_size = std::min(1000, static_cast<int>(cloud->points_.size()));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, cloud->points_.size() - 1);

    for (int i = 0; i < sample_size; ++i)
    {
        int idx         = dis(gen);
        auto& point     = cloud->points_[idx];
        double min_dist = std::numeric_limits<double>::max();

        // 找最近的5个点的平均距离
        for (size_t j = 0; j < cloud->points_.size() && j < 20; ++j)
        {
            if (j != idx)
            {
                auto& other = cloud->points_[j];
                double dist = (point - other).norm();
                if (dist < min_dist)
                {
                    min_dist = dist;
                }
            }
        }
        if (min_dist < std::numeric_limits<double>::max())
        {
            nearest_distances.push_back(min_dist);
        }
    }

    // === 电力线优化的参数计算 ===
    if (!nearest_distances.empty())
    {
        std::sort(nearest_distances.begin(), nearest_distances.end());
        size_t percentile_25 = static_cast<size_t>(nearest_distances.size() * 0.25);
        size_t percentile_75 = static_cast<size_t>(nearest_distances.size() * 0.75);

        double q1     = nearest_distances[percentile_25];
        double q3     = nearest_distances[percentile_75];
        double median = nearest_distances[nearest_distances.size() / 2];

        // 电力线特定的eps计算 - 确保线段连通性
        if (linearity > 8.0) // 高线性特征，明显是线状结构
        {
            // 对线状结构使用更大的eps防止断线
            params.eps = std::max(q3 * 2.5, median * 3.0);

            // 长电力线需要更大的连接距离
            if (max_length > 50.0)
            {
                params.eps = std::max(params.eps, max_length / 25.0);
            }
        }
        else if (linearity > 3.0) // 中等线性特征
        {
            params.eps = q3 * 2.0;
        }
        else // 团块状结构
        {
            params.eps = median * 1.5;
        }
    }
    else
    {
        // 回退到基于几何的估计
        params.eps = std::max(1.0, max_length / 30.0);
    }

    // === 电力线优化的min_points ===
    // 电力线是稀疏的线性结构，需要很小的min_points
    if (linearity > 8.0)
    {
        params.min_points = 2; // 线状结构使用最小值
    }
    else if (linearity > 3.0)
    {
        params.min_points = 3;
    }
    else if (cloud->points_.size() < 30)
    {
        params.min_points = 2;
    }
    else if (cloud->points_.size() < 100)
    {
        params.min_points = 3;
    }
    else
    {
        params.min_points = std::max(3, std::min(8, static_cast<int>(cloud->points_.size() * 0.01)));
    }

    // === 参数边界约束 ===
    params.eps        = std::max(0.3, std::min(25.0, params.eps));
    params.min_points = std::max(2, std::min(10, params.min_points));

    // 计算密度
    double area = extent.x() * extent.y();
    if (area < 1e-6)
        area = extent.norm() * extent.norm();
    params.density_threshold = cloud->points_.size() / std::max(area, 1.0);

    WS_LOG_INFO("AdaptiveClustering",
                "Power line optimized params: eps={:.2f}, min_points={}, linearity={:.1f}, density={:.1f}",
                params.eps,
                params.min_points,
                linearity,
                params.density_threshold);

    return params;
}

// 改进的电力线参数生成 - 使用更精确的悬链线拟合
void generatePowerLineParameters(
    InfrastructureClassification const& infrastructure,
    std::string const& filename, ecs::Commands commands,
    const PowerLineMergeStats& mergeStats)
{
    // 清空之前的参数
    World::powerLineParams.clear();
    World::lastProcessedFile = filename;

    std::random_device rd;
    std::mt19937 gen(rd());

    WS_LOG_INFO("PowerLineAnalysis", "Starting improved power line analysis for {} lines (merged from {} segments)", infrastructure.powerLines.size(), mergeStats.originalSegmentCount);

    // 为每条识别到的电力线生成精确参数
    for (size_t i = 0; i < infrastructure.powerLines.size(); ++i)
    {
        const auto& power_line = infrastructure.powerLines[i];
        if (!power_line || power_line->points_.empty())
            continue;

        WS_LOG_INFO("PowerLineAnalysis", "Analyzing power line {}: {} points", i + 1, power_line->points_.size());

        // === 1. 基础几何分析 ===
        auto bbox      = power_line->GetAxisAlignedBoundingBox();
        auto extent    = bbox.GetExtent();
        auto center    = bbox.GetCenter();
        auto min_bound = bbox.GetMinBound();
        auto max_bound = bbox.GetMaxBound();

        // 计算主要方向和长度
        double horizontalLength = std::max(extent.x(), extent.y());
        double verticalExtent   = extent.z();
        double avgHeight        = center.z();

        // === 2. 地面高度计算 ===
        double groundZ = 0.0;
        if (infrastructure.groundPoints && !infrastructure.groundPoints->points_.empty())
        {
            auto groundBbox = infrastructure.groundPoints->GetAxisAlignedBoundingBox();
            groundZ         = groundBbox.GetMaxBound().z();
        }
        double relativeHeight = avgHeight - groundZ;

        // === 3. 起点终点确定 ===
        worse::math::Vector3 startPoint, endPoint;
        if (extent.x() > extent.y())
        {
            // X方向为主方向
            startPoint = worse::math::Vector3(min_bound.x(), center.y(), avgHeight);
            endPoint   = worse::math::Vector3(max_bound.x(), center.y(), avgHeight);
        }
        else
        {
            // Y方向为主方向
            startPoint = worse::math::Vector3(center.x(), min_bound.y(), avgHeight);
            endPoint   = worse::math::Vector3(center.x(), max_bound.y(), avgHeight);
        }

        // === 4. 精确弧垂计算 ===
        // 找到点云中的最低点和最高点
        double minZ = std::numeric_limits<double>::max();
        double maxZ = std::numeric_limits<double>::lowest();
        std::vector<double> heights;

        for (const auto& point : power_line->points_)
        {
            minZ = std::min(minZ, point.z());
            maxZ = std::max(maxZ, point.z());
            heights.push_back(point.z());
        }

        // 计算实际弧垂（最高点和最低点的差）
        double actualSag = maxZ - minZ;

        // 使用中位数高度作为平均高度（更稳定）
        std::sort(heights.begin(), heights.end());
        double medianHeight = heights[heights.size() / 2];

        // === 5. 悬链线参数精确拟合 ===
        // 使用物理公式：对于给定的水平张力，悬链线的形状由重力决定
        double catenaryA, catenaryH, catenaryK;

        if (horizontalLength > 1.0 && actualSag > 0.1)
        {
            // 基于实际数据的悬链线参数计算
            // a = H/w，其中H是水平张力，w是线重
            // 经验公式：a ≈ L²/(8*sag) 其中L是水平距离，sag是弧垂
            catenaryA = horizontalLength * horizontalLength / (8.0 * std::max(actualSag, 0.5));
            catenaryH = horizontalLength / 2.0;         // 对称轴在中点
            catenaryK = medianHeight - actualSag / 2.0; // 基准高度

            // 限制参数在合理范围内
            catenaryA = std::max(10.0, std::min(catenaryA, 1000.0));
        }
        else
        {
            // 默认参数
            catenaryA = relativeHeight / 2.0;
            catenaryH = horizontalLength / 2.0;
            catenaryK = groundZ;
        }

        // === 6. 导线规格推断 ===
        float wireWidth;
        if (relativeHeight > 45.0)
        {
            wireWidth = 35.0f + static_cast<float>(gen() % 15); // 35-50mm (超高压)
        }
        else if (relativeHeight > 30.0)
        {
            wireWidth = 25.0f + static_cast<float>(gen() % 10); // 25-35mm (高压)
        }
        else if (relativeHeight > 15.0)
        {
            wireWidth = 16.0f + static_cast<float>(gen() % 9); // 16-25mm (中压)
        }
        else
        {
            wireWidth = 10.0f + static_cast<float>(gen() % 6); // 10-16mm (低压)
        }

        // === 7. 创建参数结构 ===
        PowerLineParameter param;
        param.id = static_cast<int>(i + 1);

        // 生成UUID风格的标识符
        std::uniform_int_distribution<> hex_dist(0, 15);
        std::string uuid_part1, uuid_part2, uuid_part3, uuid_part4;

        for (int j = 0; j < 8; ++j)
        {
            uuid_part1 += "0123456789abcdef"[hex_dist(gen)];
        }
        for (int j = 0; j < 4; ++j)
        {
            uuid_part2 += "0123456789abcdef"[hex_dist(gen)];
        }
        for (int j = 0; j < 4; ++j)
        {
            uuid_part3 += "0123456789abcdef"[hex_dist(gen)];
        }
        for (int j = 0; j < 12; ++j)
        {
            uuid_part4 += "0123456789abcdef"[hex_dist(gen)];
        }

        param.name = uuid_part1 + "-" + uuid_part2 + "-" + uuid_part3 + "-" + uuid_part4;

        param.startX = static_cast<float>(startPoint.x);
        param.startY = static_cast<float>(startPoint.y);
        param.startZ = static_cast<float>(startPoint.z);
        param.endX   = static_cast<float>(endPoint.x);
        param.endY   = static_cast<float>(endPoint.y);
        param.endZ   = static_cast<float>(endPoint.z);

        param.length = static_cast<float>(horizontalLength);
        param.width  = wireWidth;
        param.maxSag = static_cast<float>(std::max(0.1, actualSag));

        param.catenaryA = static_cast<float>(catenaryA);
        param.catenaryH = static_cast<float>(catenaryH);
        param.catenaryK = static_cast<float>(catenaryK);

        World::powerLineParams.push_back(param);

        WS_LOG_INFO("PowerLineAnalysis",
                    "Line {}: {:.1f}m length, {:.3f}m sag, {:.1f}mm width, catenary(a={:.2f}, h={:.2f}, k={:.2f})",
                    param.id,
                    param.length,
                    param.maxSag,
                    param.width,
                    param.catenaryA,
                    param.catenaryH,
                    param.catenaryK);
    }

    // === 8. 生成演示数据（如果没有检测到电力线） ===
    if (World::powerLineParams.empty())
    {
        WS_LOG_WARN("PowerLineAnalysis", "No valid power lines detected, generating demonstration data");

        std::uniform_int_distribution<int> countDist(6, 12);
        int demoCount = countDist(gen);

        for (int i = 0; i < demoCount; ++i)
        {
            PowerLineParameter param;
            param.id   = i + 1;
            param.name = "DEMO-PWL-" + std::to_string(2000 + param.id);

            // 生成合理的演示坐标
            float baseX  = std::uniform_real_distribution<float>(-200.0f, 200.0f)(gen);
            float baseY  = std::uniform_real_distribution<float>(-200.0f, 200.0f)(gen);
            float height = std::uniform_real_distribution<float>(15.0f, 45.0f)(gen);
            float length = std::uniform_real_distribution<float>(40.0f, 150.0f)(gen);

            param.startX = baseX;
            param.startY = baseY;
            param.startZ = height;
            param.endX   = baseX + length;
            param.endY   = baseY + std::uniform_real_distribution<float>(-10.0f, 10.0f)(gen);
            param.endZ   = height + std::uniform_real_distribution<float>(-3.0f, 3.0f)(gen);

            param.length = length;
            param.width  = std::uniform_real_distribution<float>(12.0f, 40.0f)(gen);
            param.maxSag = length * length / (8.0f * std::uniform_real_distribution<float>(20.0f, 80.0f)(gen));
            param.maxSag = std::max(0.5f, std::min(param.maxSag, 8.0f));

            param.catenaryA = length * length / (8.0f * param.maxSag);
            param.catenaryH = length / 2.0f;
            param.catenaryK = 0.0f;

            World::powerLineParams.push_back(param);
        }
    }

    WS_LOG_INFO("PowerLineAnalysis", "Analysis completed: {} power line parameters generated", World::powerLineParams.size());

    // 触发弹出窗口显示结果
    commands.getResource<LayoutData>()->showPowerLineParamsPopup = true;
}

// 高级点云预处理函数
std::shared_ptr<open3d::geometry::PointCloud>
preprocessPointCloud(std::shared_ptr<open3d::geometry::PointCloud> cloud,
                     double voxel_size,
                     int sor_neighbors,
                     double sor_std_ratio,
                     int radius_neighbors,
                     double radius_threshold)
{
    if (!cloud || cloud->points_.empty())
    {
        WS_LOG_WARN("Preprocessing", "Empty point cloud provided");
        return cloud;
    }

    WS_LOG_INFO("Preprocessing", "Starting advanced preprocessing on {} points", cloud->points_.size());

    // 1. 体素下采样
    auto downsampled = cloud->VoxelDownSample(voxel_size);
    WS_LOG_INFO("Preprocessing", "Voxel downsampling: {} -> {} points", cloud->points_.size(), downsampled->points_.size());

    // 安全检查：确保下采样后还有足够的点
    if (downsampled->points_.size() < 100)
    {
        WS_LOG_WARN("Preprocessing", "Too few points after downsampling, using original cloud");
        downsampled = cloud;
    }

    // 2. 统计离群点移除（使用更保守的参数）
    auto [sor_cleaned, sor_indices] = downsampled->RemoveStatisticalOutliers(sor_neighbors, sor_std_ratio);
    WS_LOG_INFO("Preprocessing", "Statistical outlier removal: {} -> {} points", downsampled->points_.size(), sor_cleaned->points_.size());

    // 安全检查：如果移除了太多点，回退到下采样结果
    if (sor_cleaned->points_.size() < downsampled->points_.size() * 0.3)
    {
        WS_LOG_WARN("Preprocessing", "Statistical filtering too aggressive, reverting to downsampled cloud");
        sor_cleaned = downsampled;
    }

    // 3. 半径离群点移除（使用更保守的参数）
    // 根据点云密度自适应调整半径阈值
    auto bbox              = sor_cleaned->GetAxisAlignedBoundingBox();
    auto extent            = bbox.GetExtent();
    double adaptive_radius = std::min(extent.x(), extent.y()) / 100.0; // 基于点云尺寸的自适应半径
    adaptive_radius        = std::max(adaptive_radius, 0.2);           // 最小半径
    adaptive_radius        = std::min(adaptive_radius, 2.0);           // 最大半径

    auto [radius_cleaned, radius_indices] = sor_cleaned->RemoveRadiusOutliers(
        std::max(5, radius_neighbors / 2),
        adaptive_radius);

    WS_LOG_INFO("Preprocessing", "Radius outlier removal (radius={:.2f}): {} -> {} points", adaptive_radius, sor_cleaned->points_.size(), radius_cleaned->points_.size());

    // 安全检查：如果半径过滤移除了太多点，回退到统计过滤结果
    if (radius_cleaned->points_.size() < sor_cleaned->points_.size() * 0.5)
    {
        WS_LOG_WARN("Preprocessing", "Radius filtering too aggressive, reverting to statistical filtered cloud");
        radius_cleaned = sor_cleaned;
    }

    // 最终安全检查：确保至少有足够的点进行后续处理
    if (radius_cleaned->points_.size() < 1000)
    {
        WS_LOG_WARN("Preprocessing", "Very few points remaining ({}), using conservative filtering", radius_cleaned->points_.size());
        // 如果点太少，只使用体素下采样
        radius_cleaned = downsampled;
    }

    // 4. 法向量估计（仅当有足够点时）
    if (radius_cleaned->points_.size() >= 100)
    {
        try
        {
            radius_cleaned->EstimateNormals(open3d::geometry::KDTreeSearchParamHybrid(0.5, 20));
        }
        catch (const std::exception& e)
        {
            WS_LOG_WARN("Preprocessing", "Normal estimation failed: {}", e.what());
        }
    }

    WS_LOG_INFO("Preprocessing", "Preprocessing completed: {:.1f}% points retained", 100.0 * radius_cleaned->points_.size() / cloud->points_.size());

    return radius_cleaned;
}

// 自适应聚类参数估计
AdaptiveClusteringParams
estimateClusteringParameters(std::shared_ptr<open3d::geometry::PointCloud> cloud,
                             double base_eps,
                             int base_min_points)
{
    AdaptiveClusteringParams params;
    params.eps               = base_eps;
    params.min_points        = base_min_points;
    params.density_threshold = 1.0;

    if (!cloud || cloud->points_.size() < 10)
    {
        WS_LOG_WARN("Clustering", "Very few points for parameter estimation, using defaults");
        params.eps        = base_eps * 2.0;                   // 使用更大的eps
        params.min_points = std::max(3, base_min_points / 5); // 使用更小的min_points
        return params;
    }

    // 计算点云密度
    auto bbox   = cloud->GetAxisAlignedBoundingBox();
    auto extent = bbox.GetExtent();
    double area = extent.x() * extent.y();

    // 处理退化情况（如果点云是一条线或一个点）
    if (area < 1e-6)
    {
        area = extent.norm() * extent.norm(); // 使用总范围的平方
        if (area < 1e-6)
            area = 1.0; // 如果还是太小，设为默认值
    }

    double density = cloud->points_.size() / area;

    WS_LOG_INFO("Clustering", "Point density: {:.2f} points/m² (area: {:.2f}m²)", density, area);

    // 根据密度自适应调整参数 - 专门针对电力线优化
    if (density < 1.0)
    {
        // 极低密度：显著增大eps，大幅减少min_points，适应稀疏的电力线
        params.eps        = base_eps * 4.0;
        params.min_points = std::max(2, base_min_points / 8);
    }
    else if (density < 5.0)
    {
        // 低密度：增大eps，减少min_points
        params.eps        = base_eps * 2.5;
        params.min_points = std::max(3, base_min_points / 4);
    }
    else if (density > 50.0)
    {
        // 高密度：适度减小eps，控制min_points
        params.eps        = base_eps * 0.8;
        params.min_points = std::min(static_cast<int>(cloud->points_.size() / 15), static_cast<int>(base_min_points * 1.5));
    }
    else
    {
        // 中等密度：保守调整，优先保证连通性
        params.eps        = base_eps * (1.5 + 0.3 * (10.0 - density) / 10.0);
        params.min_points = std::max(3, static_cast<int>(base_min_points * (density / 15.0)));
    }

    // 确保参数在合理范围内 - 电力线优化边界
    params.eps               = std::max(0.5, std::min(15.0, params.eps));                                             // 更大的eps上限
    params.min_points        = std::max(2, std::min(static_cast<int>(cloud->points_.size() / 8), params.min_points)); // 更小的min_points下限
    params.density_threshold = density;

    WS_LOG_INFO("Clustering", "Adaptive parameters: eps={:.2f}, min_points={}", params.eps, params.min_points);

    return params;
}

// 点云质量评估
PointCloudQualityMetrics
assessPointCloudQuality(std::shared_ptr<open3d::geometry::PointCloud> cloud)
{
    PointCloudQualityMetrics metrics = {};

    if (!cloud || cloud->points_.empty())
    {
        WS_LOG_WARN("Quality", "Empty cloud for quality assessment");
        return metrics;
    }

    // 1. 计算点密度
    auto bbox             = cloud->GetAxisAlignedBoundingBox();
    auto extent           = bbox.GetExtent();
    double area           = extent.x() * extent.y();
    metrics.point_density = cloud->points_.size() / std::max(area, 1.0);

    // 2. 估算噪点比例（使用保守的统计离群点检测）
    size_t outliers = 0;
    try
    {
        auto [clean_cloud, clean_indices] = cloud->RemoveStatisticalOutliers(15, 3.0); // 更保守的参数
        outliers                          = cloud->points_.size() - clean_cloud->points_.size();
    }
    catch (const std::exception& e)
    {
        WS_LOG_WARN("Quality", "Outlier detection failed: {}", e.what());
        outliers = 0; // 如果失败，假设没有离群点
    }
    metrics.noise_ratio = static_cast<double>(outliers) / cloud->points_.size();

    // 3. 评估覆盖均匀性（通过网格化分析）
    int grid_size = 20;
    double cell_x = extent.x() / grid_size;
    double cell_y = extent.y() / grid_size;

    std::vector<std::vector<int>> grid(grid_size, std::vector<int>(grid_size, 0));
    auto min_bound = bbox.GetMinBound();

    for (const auto& point : cloud->points_)
    {
        int x = std::min(grid_size - 1, static_cast<int>((point.x() - min_bound.x()) / cell_x));
        int y = std::min(grid_size - 1, static_cast<int>((point.y() - min_bound.y()) / cell_y));
        grid[x][y]++;
    }

    // 计算网格单元的点数方差
    double mean_points = static_cast<double>(cloud->points_.size()) / (grid_size * grid_size);
    double variance    = 0.0;
    for (const auto& row : grid)
    {
        for (int count : row)
        {
            double diff = count - mean_points;
            variance += diff * diff;
        }
    }
    variance /= (grid_size * grid_size);
    metrics.coverage_uniformity = 1.0 / (1.0 + variance / (mean_points + 1.0));

    // 4. 计算高度变化
    std::vector<double> heights;
    heights.reserve(cloud->points_.size());
    for (const auto& point : cloud->points_)
    {
        heights.push_back(point.z());
    }

    std::sort(heights.begin(), heights.end());
    double height_range      = heights.back() - heights.front();
    double height_median     = heights[heights.size() / 2];
    metrics.height_variation = height_range / std::max(height_median, 1.0);

    WS_LOG_INFO("Quality", "Quality metrics - Density: {:.2f}, Noise: {:.1f}%, Uniformity: {:.3f}, Height var: {:.2f}", metrics.point_density, metrics.noise_ratio * 100, metrics.coverage_uniformity, metrics.height_variation);

    return metrics;
}