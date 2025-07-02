#pragma once
#include "Log.hpp"
#include "Math/Base.hpp"
#include "Math/Math.hpp"
#include "Math/BoundingBox.hpp"
#include "Math/Transform.hpp"
#include "Profiling/Stopwatch.hpp"
#include "RHITypes.hpp"

#include "lasreader.hpp"

#include <vector>
#include <filesystem>

namespace worse::pc
{

    struct PointCloud
    {
        // 变换处理后的点云数据
        std::vector<RHIVertexPosUvNrmTan> points;
        // 变换后的点云包围盒
        math::BoundingBox boundingBox;

        math::BoundingBox boundingBoxOrigin;
    };

    static PointCloud load(std::filesystem::path const& filePath)
    {
        PointCloud cloud;

        LASreader* reader = nullptr;
        LASreadOpener opener;
        opener.set_file_name(filePath.string().c_str());
        reader = opener.open();

        WS_LOG_INFO("LAS", "Open {} successfully", opener.get_file_name());

        cloud.points.reserve(reader->npoints);

        cloud.boundingBoxOrigin =
            math::BoundingBox{{static_cast<float>(reader->get_min_x()),
                               static_cast<float>(reader->get_min_y()),
                               static_cast<float>(reader->get_min_z())},
                              {static_cast<float>(reader->get_max_x()),
                               static_cast<float>(reader->get_max_y()),
                               static_cast<float>(reader->get_max_z())}};

        WS_LOG_INFO("LAS",
                    "Point range: min({:.3f}, {:.3f}, {:.3f}), "
                    "max({:.3f}, {:.3f}, {:.3f})",
                    cloud.boundingBoxOrigin.getMin().x,
                    cloud.boundingBoxOrigin.getMin().y,
                    cloud.boundingBoxOrigin.getMin().z,
                    cloud.boundingBoxOrigin.getMax().x,
                    cloud.boundingBoxOrigin.getMax().y,
                    cloud.boundingBoxOrigin.getMax().z);

        math::Vector3 const center = cloud.boundingBoxOrigin.getCenter();
        math::Vector3 const extent = cloud.boundingBoxOrigin.getExtent();
        float maxExtent            = extent.elementMax();

        // 正确的变换顺序：平移到原点 -> 缩放 -> 旋转（如果需要）
        // 先平移到原点
        math::Matrix4 translateToOrigin = math::makeTranslation(-center);

        // 然后缩放到标准化尺寸
        math::Matrix4 scale = math::makeScale(math::Vector3::ONE() / maxExtent);

        // 可选：旋转（根据需要调整坐标系）
        // 注意：-90度X轴旋转将YZ平面转换为ZY平面，确认这是否符合你的需求
        math::Matrix4 rotation = math::makeRotation(
            math::Quaternion::fromAxisAngle(math::Vector3::X(),
                                            math::toRadians(-90.0f)));

        // 组合变换：右乘顺序 = 执行顺序
        math::Matrix4 transform = rotation * scale * translateToOrigin;
        profiling::Stopwatch timer;
        while (reader->read_point())
        {
            math::Vector4 position{static_cast<float>(reader->get_x()),
                                   static_cast<float>(reader->get_y()),
                                   static_cast<float>(reader->get_z()),
                                   1.0f};

            cloud.points.emplace_back((transform * position).xyz());
        }

        cloud.boundingBox = math::BoundingBox{
            (cloud.boundingBoxOrigin.getMin() - center) / maxExtent,
            (cloud.boundingBoxOrigin.getMax() - center) / maxExtent};

        WS_LOG_INFO("LAS",
                    "Read {} points in {:.3f} ms, from ({:.3f}, {:.3f}, "
                    "{:.3f}) to ({:.3f}, {:.3f}, {:.3f})",
                    cloud.points.size(),
                    timer.elapsedMs(),
                    cloud.boundingBox.getMin().x,
                    cloud.boundingBox.getMin().y,
                    cloud.boundingBox.getMin().z,
                    cloud.boundingBox.getMax().x,
                    cloud.boundingBox.getMax().y,
                    cloud.boundingBox.getMax().z);

        return cloud;
    }

} // namespace worse::pc