#pragma once

#include "Math/BoundingBox.hpp"
#include "Math/Vector.hpp"
#include <memory>
#include <vector>

// Forward declaration
namespace open3d
{
    namespace geometry
    {
        class PointCloud;
    }
} // namespace open3d

// 电力基础设施分类结果结构
struct InfrastructureClassification
{
    std::shared_ptr<open3d::geometry::PointCloud> ground_points;
    std::shared_ptr<open3d::geometry::PointCloud> tower_points;
    std::vector<std::shared_ptr<open3d::geometry::PointCloud>> power_lines;
    worse::math::BoundingBox tower_bbox;
    std::vector<std::vector<worse::math::Vector3>> power_line_curves;
};
