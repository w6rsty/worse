#pragma once

// 临时解决fmt consteval问题
#define FMT_CONSTEVAL

#include "Math/Math.hpp"

#include <open3d/Open3D.h>

#include <memory>
#include <vector>

// Forward declaration
struct PowerLineMergeStats;

// 电力基础设施分类结果结构
struct InfrastructureClassification
{
    std::shared_ptr<open3d::geometry::PointCloud> groundPoints;
    std::shared_ptr<open3d::geometry::PointCloud> towerPoints;
    std::vector<std::shared_ptr<open3d::geometry::PointCloud>> powerLines;
    worse::math::Vector3 towerMin;
    worse::math::Vector3 towerMax;
    std::vector<std::vector<worse::math::Vector3>> powerLineCurves;

    // 合并统计信息
    PowerLineMergeStats* mergeStats = nullptr;
};
