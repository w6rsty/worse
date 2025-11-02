#pragma once
#include "math/hash.hpp"
#include "RHIDefinitions.hpp"

#include <functional>

namespace Worse
{

    class RHIRasterizerState
    {
    public:
        // clang-format off
        constexpr RHIRasterizerState(
            RHIPolygonMode const polygonMode = RHIPolygonMode::Solid,
            RHICullMode const cullMode       = RHICullMode::None,
            RHIFrontFace const frontFace     = RHIFrontFace::CCW,
            Float const depthBias            = 0.0f,
            Float const depthBiasSlopeFactor = 1.0f,
            Bool const depthClampEnable      = false,
            Float const depthBiasClamp       = 0.0f)
        {
            m_polygonMode          = polygonMode;
            m_cullMode             = cullMode;
            m_frontFace            = frontFace;

            // finalDepth = originalDepth + depthBiasSlopeFactor * maxSlope + depthBias
            m_depthBias            = depthBias;
            m_depthBiadSlopeFactor = depthBiasSlopeFactor;

            m_depthClampEnable     = depthClampEnable;
            m_depthBiasClamp       = depthBiasClamp;

            std::hash<Float> hasher;

            m_hash = math::hashCombine(m_hash, static_cast<ULong>(polygonMode));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(cullMode));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(frontFace));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(depthClampEnable));
            m_hash = math::hashCombine(m_hash, hasher(depthBias));
            m_hash = math::hashCombine(m_hash, hasher(depthBiasClamp));
            m_hash = math::hashCombine(m_hash, hasher(depthBiasSlopeFactor));
        }

        RHIPolygonMode getPolygonMode() const          { return m_polygonMode; }
        RHICullMode    getCullMode() const             { return m_cullMode; }
        RHIFrontFace   getFrontFace() const            { return m_frontFace; }
        Bool           getDepthClampEnable() const     { return m_depthClampEnable; }
        Float            getDepthBias() const            { return m_depthBias; }
        Float            getDepthBiasClamp() const       { return m_depthBiasClamp; }
        Float            getDepthBiasSlopeFactor() const { return m_depthBiadSlopeFactor; }
        ULong  getHash() const                 { return m_hash; }

        Bool operator==(RHIRasterizerState const& other) const { return m_hash == other.m_hash; }
        Bool operator!=(RHIRasterizerState const& other) const { return m_hash != other.m_hash; }
        // clang-format on

    private:
        RHIPolygonMode m_polygonMode;
        RHICullMode m_cullMode;
        RHIFrontFace m_frontFace;
        Bool m_depthClampEnable;
        Float m_depthBias;
        Float m_depthBiasClamp;
        Float m_depthBiadSlopeFactor;

        ULong m_hash = 0;
    };
} // namespace Worse