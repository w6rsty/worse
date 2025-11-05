#pragma once
#include "RHIResource.hpp"

namespace Worse
{

    class RHISampler : public RHIResource
    {
        void nativeCreate();

    public:
        RHISampler(
            RHIFilter filterMin,
            RHIFilter filterMag,
            RHIFilter filterMipmap,
            RHISamplerAddressMode addressMode,
            RHICompareOperation compareOp = RHICompareOperation::Never,
            Bool enableCompare = false, Float anisotroy = 0.0f,
            Float mipLodBias = 0.0f);
        ~RHISampler();

        // clang-format off
        RHIFilter             getFilterMin() const     { return m_filterMin; }
        RHIFilter             getFilterMag() const     { return m_filterMag; }
        RHIFilter             getFilterMipmap() const  { return m_filterMipmap; }
        RHISamplerAddressMode getAddressMode() const   { return m_addressMode; }
        RHICompareOperation   getCompareOp() const     { return m_compareOp; }
        Bool                  isCompareEnabled() const { return m_enableCompare; }
        Float                   getAnisotroy() const     { return m_anisotroy; }
        RHINativeHandle       getHandle() const        { return m_handle; }
        // clang-format on

    private:
        RHIFilter m_filterMin;
        RHIFilter m_filterMag;
        RHIFilter m_filterMipmap;
        RHISamplerAddressMode m_addressMode;

        // for SamplerComprisonState
        RHICompareOperation m_compareOp;
        Bool m_enableCompare;

        Float m_anisotroy;
        Float m_mipLodBias;

        RHINativeHandle m_handle = {};
    };

} // namespace Worse