#pragma once
#include "Math/Hash.hpp"
#include "RHIDefinitions.hpp"

#include <functional>
#include <cstdint>

namespace worse
{

    class RHIBlendState
    {
    public:
        // clang-format off
        RHIBlendState(
            bool const blendEnable               = false,
            RHIBlendFactor const srcBlend        = RHIBlendFactor::SrcAlpha,
            RHIBlendFactor const dstBlend        = RHIBlendFactor::OneMinusSrcAlpha,
            RHIBlendOperation const blendOp      = RHIBlendOperation::Add,
            RHIBlendFactor const srcAlphaBlend   = RHIBlendFactor::One,
            RHIBlendFactor const dstAlphaBlend   = RHIBlendFactor::One,
            RHIBlendOperation const alphaBlendOp = RHIBlendOperation::Add,
            float const blendFactor              = 1.0f
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

            std::hash<float> hasher;
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(blendEnable));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(srcBlend));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(dstBlend));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(blendOp));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(srcAlphaBlend));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(dstAlphaBlend));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(alphaBlendOp));
            m_hash = math::hashCombine(m_hash, hasher(blendFactor));
        }

        bool              getBlendEnable() const   { return m_blendEnable; }
        RHIBlendFactor    getSrcBlend() const      { return m_srcBlend; }
        RHIBlendFactor    getDstBlend() const      { return m_dstBlend; }
        RHIBlendOperation getBlendOp() const       { return m_blendOp; }
        RHIBlendFactor    getSrcAlphaBlend() const { return m_srcAlphaBlend; }
        RHIBlendFactor    getDstAlphaBlend() const { return m_dstAlphaBlend; }
        RHIBlendOperation getAlphaBlendOp() const  { return m_alphaBlendOp; }
        float             getBlendFactor() const   { return m_blendFactor; }
        std::uint64_t     getHash() const          { return m_hash; }

        bool operator==(RHIBlendState const& other) const { return m_hash == other.m_hash; }
        bool operator!=(RHIBlendState const& other) const { return m_hash != other.m_hash; }
        // clang-format on

    private:
        bool m_blendEnable;
        RHIBlendFactor m_srcBlend;
        RHIBlendFactor m_dstBlend;
        RHIBlendOperation m_blendOp;
        RHIBlendFactor m_srcAlphaBlend;
        RHIBlendFactor m_dstAlphaBlend;
        RHIBlendOperation m_alphaBlendOp;
        float m_blendFactor;

        std::uint64_t m_hash = 0;
    };

} // namespace worse