#pragma once
#include "RHIDefinitions.hpp"
#include "Math/Hash.hpp"

#include <cstdint>

namespace worse
{

    class RHIDepthStencilState
    {
    public:
        // clang-format off
        constexpr RHIDepthStencilState(
            bool const depthTestEnable                   = false,
            bool const depthWriteEnable                  = false,
            RHICompareOperation const depthCompareOp     = RHICompareOperation::Less,
            bool const stencilTestEnable                 = false,
            bool const stencilWriteEnable                = false,
            RHICompareOperation const stencilCompareOp   = RHICompareOperation::Always,
            RHIStencilOperation const stencilFailOp      = RHIStencilOperation::Keep,
            RHIStencilOperation const stencilDepthFailOp = RHIStencilOperation::Keep,
            RHIStencilOperation const stencilPassOp      = RHIStencilOperation::Replace,
            std::uint8_t const stencilReadMask           = 0x1,
            std::uint8_t const stencilWriteMask          = 0x1
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

            // clang-format off
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(depthTestEnable));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(depthWriteEnable));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(depthCompareOp));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(stencilTestEnable));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(stencilWriteEnable));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(stencilCompareOp));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(stencilFailOp));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(stencilDepthFailOp));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(stencilPassOp));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(stencilReadMask));
            m_hash = math::hashCombine(m_hash, static_cast<std::uint64_t>(stencilWriteMask));
            // clang-format on
        }

        bool getDepthTestEnabled() const
        {
            return m_depthTestEnable;
        }
        bool getDepthWriteEnabled() const
        {
            return m_depthWriteEnable;
        }
        RHICompareOperation getDepthCompareOp() const
        {
            return m_depthCompareOp;
        }
        bool getStencilTestEnabled() const
        {
            return m_stencilTestEnable;
        }
        bool getStencilWriteEnabled() const
        {
            return m_stencilWriteEnable;
        }
        RHICompareOperation getStencilCompareOp() const
        {
            return m_stencilCompareOp;
        }
        RHIStencilOperation getStencilFailOp() const
        {
            return m_stencilFailOp;
        }
        RHIStencilOperation getStencilDepthFailOp() const
        {
            return m_stencilDepthFailOp;
        }
        RHIStencilOperation getStencilPassOp() const
        {
            return m_stencilPassOp;
        }
        std::uint8_t getStencilReadMask() const
        {
            return m_stencilReadMask;
        }
        std::uint8_t getStencilWriteMask() const
        {
            return m_stencilWriteMask;
        }
        std::uint64_t getHash() const
        {
            return m_hash;
        }

        bool operator==(RHIDepthStencilState const& other) const
        {
            return m_hash == other.m_hash;
        }
        bool operator!=(RHIDepthStencilState const& other) const
        {
            return m_hash != other.m_hash;
        }
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
        std::uint8_t m_stencilReadMask;
        std::uint8_t m_stencilWriteMask;

        std::uint64_t m_hash = 0;
    };

} // namespace worse