#pragma once
#include "Math/Math.hpp"
#include "RHIResource.hpp"

#include <vector>

namespace worse
{

    enum class RHIVertexType
    {
        None,
        Pos,
        PosCol,
        PosUv,
        PosUvNrmTan,
    };

    inline std::uint32_t RHIVertexSize(RHIVertexType type)
    {
        switch (type)
        {
        case RHIVertexType::Pos:
            return sizeof(RHIVertexPos);
        case RHIVertexType::PosCol:
            return sizeof(RHIVertexPosCol);
        case RHIVertexType::PosUv:
            return sizeof(RHIVertexPosUv);
        case RHIVertexType::PosUvNrmTan:
            return sizeof(RHIVertexPosUvNrmTan);
        default:
            return 0;
        }
    }

    struct RHIVertexAttribute
    {
        std::string name;
        std::uint32_t location;
        std::uint32_t binding;
        RHIFormat format;
        std::uint32_t offset;
    };

    class RHIInputLayout : public RHIResource
    {
    public:
        RHIInputLayout() = default;
        explicit RHIInputLayout(RHIVertexType const type)
        {
            m_type = type;

            // clang-format off
            if (m_type == RHIVertexType::Pos)
            {
                m_stride     = sizeof(RHIVertexPos);
                m_attributes = {
                    {"POSITION", 0, 0, RHIFormat::R32G32B32Float, offsetof(RHIVertexPos, position)}
                };
            }
            else if (m_type == RHIVertexType::PosCol)
            {
                m_stride     = sizeof(RHIVertexPosCol);
                m_attributes = {
                    {"POSITION", 0, 0, RHIFormat::R32G32B32Float,    offsetof(RHIVertexPosCol, position)},
                    {"COLOR",    1, 0, RHIFormat::R32G32B32A32Float, offsetof(RHIVertexPosCol, color)}
                };
            }
            else if (m_type == RHIVertexType::PosUv)
            {
                m_stride     = sizeof(RHIVertexPosUv);
                m_attributes = {
                    {"POSITION", 0, 0, RHIFormat::R32G32B32Float, offsetof(RHIVertexPosUv, position)},
                    {"UV",       1, 0, RHIFormat::R32G32Float,    offsetof(RHIVertexPosUv, uv)}
                };
            }
            else if (m_type == RHIVertexType::PosUvNrmTan)
            {
                m_stride     = sizeof(RHIVertexPosUvNrmTan);
                m_attributes = {
                    {"POSITION", 0, 0, RHIFormat::R32G32B32Float, offsetof(RHIVertexPosUvNrmTan, position)},
                    {"UV",       1, 0, RHIFormat::R32G32Float,    offsetof(RHIVertexPosUvNrmTan, uv)},
                    {"NORMAL",   2, 0, RHIFormat::R32G32B32Float, offsetof(RHIVertexPosUvNrmTan, normal)},
                    {"TANGENT",  3, 0, RHIFormat::R32G32B32Float, offsetof(RHIVertexPosUvNrmTan, tangent)}
                };
            }
            // clang-format on
        }

        // clang-format off
        RHIVertexType                          getType() const       { return m_type; }
        std::vector<RHIVertexAttribute> const& getAttributes() const { return m_attributes; }
        std::uint32_t                          getStride() const     { return m_stride; }
        
        bool operator==(RHIInputLayout const& other) const { return m_type == other.m_type; }
        bool operator!=(RHIInputLayout const& other) const { return !(*this == other); }
        // clang-format on

    private:
        RHIVertexType m_type = RHIVertexType::None;
        std::vector<RHIVertexAttribute> m_attributes;
        std::uint32_t m_stride = 0;
    };

} // namespace worse