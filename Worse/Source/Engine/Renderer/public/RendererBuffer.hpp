#pragma once
#include "math/math_includes.hpp"
#include "RHIDefinitions.hpp"

#include <span>

namespace Worse
{

    class FrameConstantData
    {
    public:
        Float deltaTime = 0.0f;
        Float time      = 0.0f;
        Float cameraNear;
        Float cameraFar;

        Vector3 cameraPosition;
        Float padding0;

        Vector3 cameraForward;
        Float padding1;

        Matrix4 view;
        Matrix4 projection;
        Matrix4 viewProjection;
        Matrix4 viewProjectionInverse;
    };

    class PushConstantData
    {
    public:
        Matrix4 transform = Matrix4::IDENTITY;

        union
        {
            struct
            {
                Vector2 f2        = Vector2::ZERO;
                Vector3 f30       = Vector3::ZERO;
                Vector3 f31       = Vector3::ZERO;
                Vector4 f4        = Vector4::ZERO;
                Float materialId  = 0;
                Float transparent = 0; // 0 = false, 1 = true
                Float padding[2];
            } values;

            Matrix4 matrix;
        };

        std::span<Byte, RHIConfig::MAX_PUSH_CONSTANT_SIZE> asSpan()
        {
            return std::span<Byte, RHIConfig::MAX_PUSH_CONSTANT_SIZE>(
                reinterpret_cast<Byte*>(this),
                sizeof(PushConstantData));
        }

        // clang-format off
        PushConstantData& setTransform(Matrix4 const& mat)  { transform = mat; return *this; }
        PushConstantData& setMatrix(Matrix4 const& mat)     { matrix = mat; return *this; }
        PushConstantData& setF2(Vector2 const& f2)          { values.f2 = f2; return *this; }
        PushConstantData& setF30(Vector3 const& f30)        { values.f30 = f30; return *this; }
        PushConstantData& setF31(Vector3 const& f31)        { values.f31 = f31; return *this; }
        PushConstantData& setF4(Vector4 const& f4)          { values.f4 = f4; return *this; }
        PushConstantData& setMaterialId(UInt const id)      { values.materialId = static_cast<UInt>(id); return *this; }
        PushConstantData& setTransparent(Bool const enable) { values.transparent = enable ? 1.0f : 0.0f; return *this; }
        PushConstantData& setPadding(Float const p0, Float const p1)
        {
            values.padding[0] = p0;
            values.padding[1] = p1;
            return *this;
        }
        // clang-format on
    };

} // namespace Worse
