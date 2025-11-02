#pragma once
#include "math/hash.hpp"
#include "RHIDefinitions.hpp"

#include <functional>

namespace Worse
{

    class RHIBlendState
    {
    public:
        // clang-format off
        RHIBlendState(
            Bool const blendEnable               = false,
            RHIBlendFactor const srcBlend        = RHIBlendFactor::SrcAlpha,
            RHIBlendFactor const dstBlend        = RHIBlendFactor::OneMinusSrcAlpha,
            RHIBlendOperation const blendOp      = RHIBlendOperation::Add,
            RHIBlendFactor const srcAlphaBlend   = RHIBlendFactor::One,
            RHIBlendFactor const dstAlphaBlend   = RHIBlendFactor::One,
            RHIBlendOperation const alphaBlendOp = RHIBlendOperation::Add,
            Float const blendFactor              = 1.0f
        )
        {
            m_blendEnable   = blendEnable;
            m_srcBlend      = srcBlend;
            m_dstBlend      = dstBlend;
            m_blendOp       = blendOp;
            m_srcAlphaBlend = srcAlphaBlend;
            m_dstAlphaBlend = dstAlphaBlend;
            m_alphaBlendOp  = alphaBlendOp;
            m_blendFactor   = blendFactor;

            std::hash<Float> hasher;
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(blendEnable));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(srcBlend));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(dstBlend));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(blendOp));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(srcAlphaBlend));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(dstAlphaBlend));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(alphaBlendOp));
            m_hash = math::hashCombine(m_hash, hasher(blendFactor));
        }

        Bool              getBlendEnable() const   { return m_blendEnable; }
        RHIBlendFactor    getSrcBlend() const      { return m_srcBlend; }
        RHIBlendFactor    getDstBlend() const      { return m_dstBlend; }
        RHIBlendOperation getBlendOp() const       { return m_blendOp; }
        RHIBlendFactor    getSrcAlphaBlend() const { return m_srcAlphaBlend; }
        RHIBlendFactor    getDstAlphaBlend() const { return m_dstAlphaBlend; }
        RHIBlendOperation getAlphaBlendOp() const  { return m_alphaBlendOp; }
        Float             getBlendFactor() const   { return m_blendFactor; }
        ULong     getHash() const          { return m_hash; }

        Bool operator==(RHIBlendState const& other) const { return m_hash == other.m_hash; }
        Bool operator!=(RHIBlendState const& other) const { return m_hash != other.m_hash; }
        // clang-format on

    private:
        Bool m_blendEnable;
        RHIBlendFactor m_srcBlend;
        RHIBlendFactor m_dstBlend;
        RHIBlendOperation m_blendOp;
        RHIBlendFactor m_srcAlphaBlend;
        RHIBlendFactor m_dstAlphaBlend;
        RHIBlendOperation m_alphaBlendOp;
        Float m_blendFactor;

        ULong m_hash = 0;
    };

} // namespace Worse