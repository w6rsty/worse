#pragma once
#include "base_type.hpp"
#include "math/hash.hpp"
#include "RHIDefinitions.hpp"

namespace Worse
{

    class RHIDepthStencilState
    {
    public:
        // clang-format off
        constexpr RHIDepthStencilState(
            Bool const depthTestEnable                   = false,
            Bool const depthWriteEnable                  = false,
            RHICompareOperation const depthCompareOp     = RHICompareOperation::GreaterEqual,
            Bool const stencilTestEnable                 = false,
            Bool const stencilWriteEnable                = false,
            RHICompareOperation const stencilCompareOp   = RHICompareOperation::Equal,
            RHIStencilOperation const stencilFailOp      = RHIStencilOperation::Keep,
            RHIStencilOperation const stencilDepthFailOp = RHIStencilOperation::Keep,
            RHIStencilOperation const stencilPassOp      = RHIStencilOperation::Replace,
            UByte const stencilReadMask                  = 0x1,
            UByte const stencilWriteMask                 = 0x1
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

            m_hash = math::hashCombine(m_hash, static_cast<ULong>(depthTestEnable));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(depthWriteEnable));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(depthCompareOp));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(stencilTestEnable));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(stencilWriteEnable));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(stencilCompareOp));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(stencilFailOp));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(stencilDepthFailOp));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(stencilPassOp));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(stencilReadMask));
            m_hash = math::hashCombine(m_hash, static_cast<ULong>(stencilWriteMask));
        }

        Bool                getDepthTestEnabled() const    { return m_depthTestEnable; }
        Bool                getDepthWriteEnabled() const   { return m_depthWriteEnable; }
        RHICompareOperation getDepthCompareOp() const      { return m_depthCompareOp; }
        Bool                getStencilTestEnabled() const  { return m_stencilTestEnable; }
        Bool                getStencilWriteEnabled() const { return m_stencilWriteEnable; }
        RHICompareOperation getStencilCompareOp() const    { return m_stencilCompareOp; }
        RHIStencilOperation getStencilFailOp() const       { return m_stencilFailOp; }
        RHIStencilOperation getStencilDepthFailOp() const  { return m_stencilDepthFailOp; }
        RHIStencilOperation getStencilPassOp() const       { return m_stencilPassOp; }
        UByte               getStencilReadMask() const     { return m_stencilReadMask; }
        UByte               getStencilWriteMask() const    { return m_stencilWriteMask; }
        ULong                 getHash() const                { return m_hash; }

        Bool operator==(RHIDepthStencilState const& other) const { return m_hash == other.m_hash; }
        Bool operator!=(RHIDepthStencilState const& other) const { return m_hash != other.m_hash; }
        // clang-format on

    private:
        Bool m_depthTestEnable;
        Bool m_depthWriteEnable;
        RHICompareOperation m_depthCompareOp;
        Bool m_stencilTestEnable;
        Bool m_stencilWriteEnable;
        RHICompareOperation m_stencilCompareOp;
        RHIStencilOperation m_stencilFailOp;
        RHIStencilOperation m_stencilDepthFailOp;
        RHIStencilOperation m_stencilPassOp;
        UByte m_stencilReadMask;
        UByte m_stencilWriteMask;

        ULong m_hash = 0;
    };

} // namespace Worse