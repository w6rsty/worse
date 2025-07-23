#include <open3d/geometry/KDTreeFlann.h>
#include <open3d/geometry/BoundingVolume.h>
#include <open3d/io/PointCloudIO.h>

#include "Process.hpp"
#include "../Application/World.hpp"

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

        // 对高空区域使用更小的eps以精确分离电力塔和线路
        double high_eps     = eps * 0.8;
        int high_min_points = std::max(5, min_points / 3);

        auto high_labels = high_cloud->ClusterDBSCAN(high_eps, high_min_points);

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
                all_clusters.push_back(cluster);
            }
        }

        WS_LOG_INFO("PowerInfrastructure", "High-altitude clusters: {}", max_high_label + 1);
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

        auto tower_bbox  = result.towerPoints->GetAxisAlignedBoundingBox();
        auto min_bound   = tower_bbox.GetMinBound();
        auto max_bound   = tower_bbox.GetMaxBound();
        result.towerBbox = math::BoundingBox{
            math::Vector3(static_cast<float>(min_bound.x()),
                          static_cast<float>(min_bound.y()),
                          static_cast<float>(min_bound.z())),
            math::Vector3(static_cast<float>(max_bound.x()),
                          static_cast<float>(max_bound.y()),
                          static_cast<float>(max_bound.z()))};

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

// === 新增的高级处理函数实现 ===

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

    // 根据密度自适应调整参数
    if (density < 2.0)
    {
        // 极低密度：大幅增大eps，减少min_points
        params.eps        = base_eps * 3.0;
        params.min_points = std::max(3, base_min_points / 5);
    }
    else if (density < 10.0)
    {
        // 低密度：增大eps，减少min_points
        params.eps        = base_eps * 2.0;
        params.min_points = std::max(5, base_min_points / 3);
    }
    else if (density > 100.0)
    {
        // 高密度：减小eps，增加min_points
        params.eps        = base_eps * 0.7;
        params.min_points = std::min(static_cast<int>(cloud->points_.size() / 10), base_min_points * 2);
    }
    else
    {
        // 中等密度：使用基础参数的微调
        params.eps        = base_eps * (1.0 + 0.2 * (25.0 - density) / 25.0);
        params.min_points = std::max(5, static_cast<int>(base_min_points * (density / 25.0)));
    }

    // 确保参数在合理范围内
    params.eps               = std::max(0.3, std::min(10.0, params.eps));
    params.min_points        = std::max(3, std::min(static_cast<int>(cloud->points_.size() / 5), params.min_points));
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