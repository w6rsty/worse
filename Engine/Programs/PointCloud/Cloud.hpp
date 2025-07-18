#pragma once
#include "Math/BoundingBox.hpp"
#include "Math/Matrix.hpp"
#include "RHITypes.hpp"

#include <vector>
#include <string_view>

namespace worse::pc
{

    /**
     * @brief 点云数据结构
     *
     * @details 使用+Z向上，右手系坐标。兼容 LAS 和 Open3D 坐标系。
     * 在渲染时需要转换坐标系
     */
    struct Cloud
    {
        // 变换处理后的点云数据
        std::vector<RHIVertexPosUvNrmTan> points;
        // 变换后的点云包围盒
        math::BoundingBox volume;

        // 变换矩阵，用于还原原始坐标
        math::Matrix4 transform = math::Matrix4::IDENTITY();
    };

    // 从 LAS 文件加载点云数据
    Cloud load(std::string_view filePath);

} // namespace worse::pc