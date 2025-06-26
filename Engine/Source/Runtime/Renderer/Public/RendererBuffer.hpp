#pragma once
#include "Math/Math.hpp"
#include "RHIDefinitions.hpp"

#include <span>

namespace worse
{

    class FrameConstantData
    {
    public:
        float deltaTime = 0.0f;
        float time      = 0.0f;
        math::Vector2 padding0; // align to 16 bytes

        math::Vector3 cameraPosition;
        float cameraNear;
        math::Vector3 cameraForward;
        float cameraFar;
        math::Vector4 padding1; // align to 16 bytes

        math::Matrix4 view;
        math::Matrix4 projection;
        math::Matrix4 viewProjection;
    };

    class PushConstantData
    {
    public:
        math::Matrix4 model = math::Matrix4::IDENTITY();

        struct MatrixZip
        {
            math::Vector2 f2  = math::Vector2::ZERO();
            math::Vector3 f30 = math::Vector3::ZERO();
            math::Vector3 f31 = math::Vector3::ZERO();
            math::Vector4 f4  = math::Vector4::ZERO();
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
        PushConstantData& setModel(math::Matrix4 const& modelMatrix)  { model = modelMatrix; return *this; }
        PushConstantData& setF2(math::Vector2 const& f2)              { values.f2 = f2; return *this; }
        PushConstantData& setF30(math::Vector3 const& f30)            { values.f30 = f30; return *this; }
        PushConstantData& setF31(math::Vector3 const& f31)            { values.f31 = f31; return *this; }
        PushConstantData& setF4(math::Vector4 const& f4)              { values.f4 = f4; return *this; }
        PushConstantData& setMaterialId(std::uint32_t const id) { values.materialId = static_cast<std::uint32_t>(id); return *this; }
        PushConstantData& setTransparent(bool const enable)     { values.transparent = enable ? 1.0f : 0.0f; return *this; }
        PushConstantData& setPadding(float const p0, float const p1)
        {
            values.padding[0] = p0;
            values.padding[1] = p1;
            return *this;
        }
        // clang-format on
    };

} // namespace worse