#pragma once
#include "Math/Hash.hpp"
#include "RHIDefinitions.hpp"

namespace worse
{

    class RHIDepthStencilState
    {
    public:
        // clang-format off
        constexpr RHIDepthStencilState(
            bool const depthTestEnable                   = false,
            bool const depthWriteEnable                  = false,
            RHICompareOperation const depthCompareOp     = RHICompareOperation::GreaterEqual,
            bool const stencilTestEnable                 = false,
            bool const stencilWriteEnable                = false,
            RHICompareOperation const stencilCompareOp   = RHICompareOperation::Equal,
            RHIStencilOperation const stencilFailOp      = RHIStencilOperation::Keep,
            RHIStencilOperation const stencilDepthFailOp = RHIStencilOperation::Keep,
            RHIStencilOperation const stencilPassOp      = RHIStencilOperation::Replace,
            u8 const stencilReadMask           = 0x1,
            u8 const stencilWriteMask          = 0x1
        )
        {
            m_depthTestEnable    = depthTestEnable;
            m_depthWriteEnable   = depthWriteEnable;
            m_depthCompareOp     = depthCompareOp;
            m_stencilTestEnable  = stencilTestEnable;
            m_stencilWriteEnable = stencilWriteEnable;
            m_stencilCompareOp   = stencilCompareOp;
            m_stencilFailOp      = stencilFailOp;
            m_stencilDepthFailOp = stencilDepthFailOp;
            m_stencilPassOp      = stencilPassOp;
            m_stencilReadMask    = stencilReadMask;
            m_stencilWriteMask   = stencilWriteMask;

            m_hash = math::hashCombine(m_hash, static_cast<u64>(depthTestEnable));
            m_hash = math::hashCombine(m_hash, static_cast<u64>(depthWriteEnable));
            m_hash = math::hashCombine(m_hash, static_cast<u64>(depthCompareOp));
            m_hash = math::hashCombine(m_hash, static_cast<u64>(stencilTestEnable));
            m_hash = math::hashCombine(m_hash, static_cast<u64>(stencilWriteEnable));
            m_hash = math::hashCombine(m_hash, static_cast<u64>(stencilCompareOp));
            m_hash = math::hashCombine(m_hash, static_cast<u64>(stencilFailOp));
            m_hash = math::hashCombine(m_hash, static_cast<u64>(stencilDepthFailOp));
            m_hash = math::hashCombine(m_hash, static_cast<u64>(stencilPassOp));
            m_hash = math::hashCombine(m_hash, static_cast<u64>(stencilReadMask));
            m_hash = math::hashCombine(m_hash, static_cast<u64>(stencilWriteMask));
        }

        bool                getDepthTestEnabled() const    { return m_depthTestEnable; }
        bool                getDepthWriteEnabled() const   { return m_depthWriteEnable; }
        RHICompareOperation getDepthCompareOp() const      { return m_depthCompareOp; }
        bool                getStencilTestEnabled() const  { return m_stencilTestEnable; }
        bool                getStencilWriteEnabled() const { return m_stencilWriteEnable; }
        RHICompareOperation getStencilCompareOp() const    { return m_stencilCompareOp; }
        RHIStencilOperation getStencilFailOp() const       { return m_stencilFailOp; }
        RHIStencilOperation getStencilDepthFailOp() const  { return m_stencilDepthFailOp; }
        RHIStencilOperation getStencilPassOp() const       { return m_stencilPassOp; }
        u8        getStencilReadMask() const     { return m_stencilReadMask; }
        u8        getStencilWriteMask() const    { return m_stencilWriteMask; }
        u64       getHash() const                { return m_hash; }

        bool operator==(RHIDepthStencilState const& other) const { return m_hash == other.m_hash; }
        bool operator!=(RHIDepthStencilState const& other) const { return m_hash != other.m_hash; }
        // clang-format on

    private:
        bool m_depthTestEnable;
        bool m_depthWriteEnable;
        RHICompareOperation m_depthCompareOp;
        bool m_stencilTestEnable;
        bool m_stencilWriteEnable;
        RHICompareOperation m_stencilCompareOp;
        RHIStencilOperation m_stencilFailOp;
        RHIStencilOperation m_stencilDepthFailOp;
        RHIStencilOperation m_stencilPassOp;
        u8 m_stencilReadMask;
        u8 m_stencilWriteMask;

        u64 m_hash = 0;
    };

} // namespace worse