#pragma once
#include "Math/Hash.hpp"
#include "RHIDefinitions.hpp"

#include <functional>

namespace worse
{

    class RHIRasterizerState
    {
    public:
        // clang-format off
        constexpr RHIRasterizerState(
            RHIPolygonMode const polygonMode = RHIPolygonMode::Solid,
            RHICullMode const cullMode       = RHICullMode::None,
            RHIFrontFace const frontFace     = RHIFrontFace::CW,
            float const lineWidth            = 1.0f,
            bool const depthClampEnable      = false,
            float const depthBias            = 0.0f,
            float const depthBiasClamp       = 0.0f,
            float const depthBiaSlopeFactor  = 0.0f)
        {
            m_polygonMode         = polygonMode;
            m_cullMode            = cullMode;
            m_frontFace           = frontFace;
            m_lineWidth           = lineWidth;
            m_depthClampEnable    = depthClampEnable;
            m_depthBias           = depthBias;
            m_depthBiasClamp      = depthBiasClamp;
            m_depthBiaSlopeFactor = depthBiaSlopeFactor;

            std::hash<float> hasher;

            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(polygonMode));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(cullMode));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(frontFace));
            m_hash = math::hashCombine(m_hash, hasher(lineWidth));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(depthClampEnable));
            m_hash = math::hashCombine(m_hash, hasher(depthBias));
            m_hash = math::hashCombine(m_hash, hasher(depthBiasClamp));
            m_hash = math::hashCombine(m_hash, hasher(depthBiaSlopeFactor));
        }

        RHIPolygonMode getPolygonMode() const          { return m_polygonMode; }
        RHICullMode    getCullMode() const             { return m_cullMode; }
        RHIFrontFace   getFrontFace() const            { return m_frontFace; }
        float          getLineWidth() const            { return m_lineWidth; }
        bool           getDepthClampEnable() const     { return m_depthClampEnable; }
        float          getDepthBias() const            { return m_depthBias; }
        float          getDepthBiasClamp() const       { return m_depthBiasClamp; }
        float          getDepthBiasSlopeFactor() const { return m_depthBiaSlopeFactor; }
        std::uint64_t  getHash() const                 { return m_hash; }

        bool operator==(RHIRasterizerState const& other) const { return m_hash == other.m_hash; }
        bool operator!=(RHIRasterizerState const& other) const { return m_hash != other.m_hash; }
        // clang-format on

    private:
        RHIPolygonMode m_polygonMode;
        RHICullMode m_cullMode;
        RHIFrontFace m_frontFace;
        float m_lineWidth;
        bool m_depthClampEnable;
        float m_depthBias;
        float m_depthBiasClamp;
        float m_depthBiaSlopeFactor;

        std::uint64_t m_hash = 0;
    };
} // namespace worse