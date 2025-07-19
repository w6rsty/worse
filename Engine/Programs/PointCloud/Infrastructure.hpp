#pragma once
#include "Math/BoundingBox.hpp"
#include "Math/Vector.hpp"

#include <open3d/geometry/PointCloud.h>

#include <memory>
#include <vector>

// 电力基础设施分类结果结构
struct InfrastructureClassification
{
    std::shared_ptr<open3d::geometry::PointCloud> groundPoints;
    std::shared_ptr<open3d::geometry::PointCloud> towerPoints;
    std::vector<std::shared_ptr<open3d::geometry::PointCloud>> powerLines;
    worse::math::BoundingBox towerBbox;
    std::vector<std::vector<worse::math::Vector3>> powerLineCurves;
};
