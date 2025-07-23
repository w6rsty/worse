#pragma once

// 临时解决fmt consteval问题
#define FMT_CONSTEVAL

#include "Math/Math.hpp"
#include <vector>
#include <string>

// Forward declarations
namespace worse::ecs
{
    class Commands;
}
struct InfrastructureClassification;

// 电力线合并统计信息
struct PowerLineMergeStats
{
    int originalSegmentCount;
    int mergedLineCount;
    std::vector<int> segmentCounts; // 每条合并线包含的原始段数
    double averageMergeQuality;
    std::string processingNote;
};

void generatePowerLineParameters(
    InfrastructureClassification const& infrastructure,
    std::string const& filename, worse::ecs::Commands commands,
    const PowerLineMergeStats& mergeStats = {});

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

// 高级点云预处理函数
std::shared_ptr<open3d::geometry::PointCloud>
preprocessPointCloud(std::shared_ptr<open3d::geometry::PointCloud> cloud,
                     double voxel_size       = 0.1,
                     int sor_neighbors       = 30,
                     double sor_std_ratio    = 1.5,
                     int radius_neighbors    = 16,
                     double radius_threshold = 0.5);

// 自适应聚类参数估计
struct AdaptiveClusteringParams
{
    double eps;
    int min_points;
    double density_threshold;
};

AdaptiveClusteringParams
estimateClusteringParameters(std::shared_ptr<open3d::geometry::PointCloud> cloud,
                             double base_eps     = 1.0,
                             int base_min_points = 50);

// 改进的电力线分析函数
AdaptiveClusteringParams calculateOptimalClusteringParams(std::shared_ptr<open3d::geometry::PointCloud> cloud);

// 断线检测和合并函数
std::pair<std::vector<std::shared_ptr<open3d::geometry::PointCloud>>, PowerLineMergeStats>
mergeBrokenPowerLines(const std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& powerLineCandidates,
                      double maxGapDistance = 20.0, double maxHeightDiff = 5.0, double maxAngleDiff = 30.0);

// 点云质量评估
struct PointCloudQualityMetrics
{
    double point_density;       // 点密度 (points/m²)
    double noise_ratio;         // 噪点比例
    double coverage_uniformity; // 覆盖均匀性
    double height_variation;    // 高度变化
};

PointCloudQualityMetrics
assessPointCloudQuality(std::shared_ptr<open3d::geometry::PointCloud> cloud);