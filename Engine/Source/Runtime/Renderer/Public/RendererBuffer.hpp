#pragma once
#include "Types.hpp"
#include "Math/Math.hpp"

namespace worse
{

    class PushConstantData
    {
    public:
        Matrix4 model = Matrix4::IDENTITY();

        //  f2.x | f30.z | f4.x | materialId
        //  f2.y | f31.x | f4.y | transparency
        // f30.x | f31.y | f4.z |
        // f30.y | f31.z | f4.w |
        struct MatrixZip
        {
            Vector2 f2               = Vector2::ZERO();
            Vector3 f30              = Vector3::ZERO();
            Vector3 f31              = Vector3::ZERO();
            Vector4 f4               = Vector4::ZERO();
            std::uint32_t materialId = 0;
            Bool32 transparency      = false;
            float padding[2];
        } values;

        PushConstantData() = default;

        PushConstantData& setModel(Matrix4 const& modelMatrix)
        {
            model = modelMatrix;
            return *this;
        }

        PushConstantData& setMaterialId(std::uint32_t const id)
        {
            values.materialId = id;
            return *this;
        }

        PushConstantData& setTransparency(bool const enable)
        {
            values.transparency = enable;
            return *this;
        }

        PushConstantData& setF2(Vector2 const& f2)
        {
            values.f2 = f2;
            return *this;
        }

        PushConstantData& setF30(Vector3 const& f30)
        {
            values.f30 = f30;
            return *this;
        }

        PushConstantData& setF31(Vector3 const& f31)
        {
            values.f31 = f31;
            return *this;
        }

        PushConstantData& setF4(Vector4 const& f4)
        {
            values.f4 = f4;
            return *this;
        }
    };

} // namespace worse