#pragma once
#include "Math/Math.hpp"
#include "RHIDefinitions.hpp"

#include <span>

namespace worse
{

    class PushConstantData
    {
    public:
        Matrix4 model = Matrix4::IDENTITY();

        struct MatrixZip
        {
            Vector2 f2        = Vector2::ZERO();
            Vector3 f30       = Vector3::ZERO();
            Vector3 f31       = Vector3::ZERO();
            Vector4 f4        = Vector4::ZERO();
            float materialId  = 0;
            float transparent = 0; // 0 = false, 1 = true
            float padding[2];
        } values;

        PushConstantData() = default;

        std::span<std::byte, RHIConfig::MAX_PUSH_CONSTANT_SIZE> asSpan()
        {
            return std::span<std::byte, RHIConfig::MAX_PUSH_CONSTANT_SIZE>(
                reinterpret_cast<std::byte*>(this),
                sizeof(PushConstantData));
        }

        // clang-format off
        PushConstantData& setModel(Matrix4 const& modelMatrix)  { model = modelMatrix; return *this; }
        PushConstantData& setF2(Vector2 const& f2)              { values.f2 = f2; return *this; }
        PushConstantData& setF30(Vector3 const& f30)            { values.f30 = f30; return *this; }
        PushConstantData& setF31(Vector3 const& f31)            { values.f31 = f31; return *this; }
        PushConstantData& setF4(Vector4 const& f4)              { values.f4 = f4; return *this; }
        PushConstantData& setMaterialId(std::uint32_t const id) { values.materialId = static_cast<std::uint32_t>(id); return *this; }
        PushConstantData& setTransparent(bool const enable)     { values.transparent = enable ? 1.0f : 0.0f; return *this; }
        // clang-format on
    };

} // namespace worse