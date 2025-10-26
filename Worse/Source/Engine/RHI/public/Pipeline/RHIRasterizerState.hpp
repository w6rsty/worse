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
            RHIFrontFace const frontFace     = RHIFrontFace::CCW,
            f32 const depthBias            = 0.0f,
            f32 const depthBiasSlopeFactor = 1.0f,
            bool const depthClampEnable      = false,
            f32 const depthBiasClamp       = 0.0f)
        {
            m_polygonMode          = polygonMode;
            m_cullMode             = cullMode;
            m_frontFace            = frontFace;

            // finalDepth = originalDepth + depthBiasSlopeFactor * maxSlope + depthBias
            m_depthBias            = depthBias;
            m_depthBiadSlopeFactor = depthBiasSlopeFactor;

            m_depthClampEnable     = depthClampEnable;
            m_depthBiasClamp       = depthBiasClamp;

            std::hash<f32> hasher;

            m_hash = math::hashCombine(m_hash, static_cast<u64>(polygonMode));
            m_hash = math::hashCombine(m_hash, static_cast<u64>(cullMode));
            m_hash = math::hashCombine(m_hash, static_cast<u64>(frontFace));
            m_hash = math::hashCombine(m_hash, static_cast<u64>(depthClampEnable));
            m_hash = math::hashCombine(m_hash, hasher(depthBias));
            m_hash = math::hashCombine(m_hash, hasher(depthBiasClamp));
            m_hash = math::hashCombine(m_hash, hasher(depthBiasSlopeFactor));
        }

        RHIPolygonMode getPolygonMode() const          { return m_polygonMode; }
        RHICullMode    getCullMode() const             { return m_cullMode; }
        RHIFrontFace   getFrontFace() const            { return m_frontFace; }
        bool           getDepthClampEnable() const     { return m_depthClampEnable; }
        f32            getDepthBias() const            { return m_depthBias; }
        f32            getDepthBiasClamp() const       { return m_depthBiasClamp; }
        f32            getDepthBiasSlopeFactor() const { return m_depthBiadSlopeFactor; }
        u64  getHash() const                 { return m_hash; }

        bool operator==(RHIRasterizerState const& other) const { return m_hash == other.m_hash; }
        bool operator!=(RHIRasterizerState const& other) const { return m_hash != other.m_hash; }
        // clang-format on

    private:
        RHIPolygonMode m_polygonMode;
        RHICullMode m_cullMode;
        RHIFrontFace m_frontFace;
        bool m_depthClampEnable;
        f32 m_depthBias;
        f32 m_depthBiasClamp;
        f32 m_depthBiadSlopeFactor;

        u64 m_hash = 0;
    };
} // namespace worse