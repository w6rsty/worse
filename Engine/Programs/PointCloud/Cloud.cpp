#include "Log.hpp"
#include "Cloud.hpp"

#include "Math/Transform.hpp"
#include "lasreader.hpp"

namespace worse::pc
{
    Cloud load(std::string_view filePath)
    {
        Cloud cloud;

        LASreader* reader = nullptr;
        LASreadOpener opener;
        opener.set_file_name(filePath.data());
        reader = opener.open();
        if (!reader)
        {
            WS_LOG_INFO("LAS", "Failed to open file: {}", filePath);
        }

        math::Vector3 minPoint{static_cast<float>(reader->get_min_x()),
                               static_cast<float>(reader->get_min_y()),
                               static_cast<float>(reader->get_min_z())};

        math::Vector3 maxPoint{static_cast<float>(reader->get_max_x()),
                               static_cast<float>(reader->get_max_y()),
                               static_cast<float>(reader->get_max_z())};

        math::Vector3 const center = (minPoint + maxPoint) * 0.5f;
        // 计算点云体积中心，将点云中心移到原点
        math::Matrix4 const translation = math::makeTranslation(-center);

        // 变换后的点云体积
        minPoint     = (translation * math::Vector4{minPoint, 1.0f}).xyz();
        maxPoint     = (translation * math::Vector4{maxPoint, 1.0f}).xyz();
        cloud.volume = math::BoundingBox{minPoint, maxPoint};

        // 开始读取点云数据
        cloud.points.reserve(reader->npoints);
        while (reader->read_point())
        {
            math::Vector3 point{static_cast<float>(reader->get_x()),
                                static_cast<float>(reader->get_y()),
                                static_cast<float>(reader->get_z())};

            cloud.points.emplace_back((translation * math::Vector4{point, 1.0f}).xyz());
        }

        // 保存变换矩阵
        cloud.transform = translation;

        return cloud;
    }

} // namespace worse::pc