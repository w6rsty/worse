#pragma once
#include "Math/Math.hpp"
#include "ECS/Commands.hpp"

#include "Infrastructure.hpp"

#include <random>

struct GroundSegmentationResult
{
    std::shared_ptr<open3d::geometry::PointCloud> groundPoints;
    std::shared_ptr<open3d::geometry::PointCloud> nonGroundPoints;
    Eigen::Vector4d groundPlaneModel; // ax + by + cz + d = 0
};

// 聚类结果结构
struct ClusteringResult
{
    std::vector<std::shared_ptr<open3d::geometry::PointCloud>> clusters;
    std::vector<int> clusterLabels;
};

// 基于高度和形状特征的精确聚类分析
ClusteringResult
performClustering(std::shared_ptr<open3d::geometry::PointCloud> cloud,
                  double eps = 0.8, int min_points = 30);
// 精确的电力基础设施分类
InfrastructureClassification classifyInfrastructure(
    const std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& clusters,
    std::shared_ptr<open3d::geometry::PointCloud> ground_points);
// 电力线曲线拟合
std::vector<worse::math::Vector3>
fitPowerLineCurve(std::shared_ptr<open3d::geometry::PointCloud> line_cloud);

// 生成简单的UUID字符串
std::string generateUUID(std::mt19937& gen);

// 生成模拟的电力线参数数据
void generatePowerLineParameters(
    InfrastructureClassification const& infrastructure,
    std::string const& filename, worse::ecs::Commands commands);

// 渐进形态滤波地面分割
GroundSegmentationResult
segmentGround(std::shared_ptr<open3d::geometry::PointCloud> cloud,
              double cellSize = 1.0, double maxWindowSize = 20.0,
              double slopeThreshold = 0.15, double maxDistance = 0.5,
              double initialDistance = 0.15);