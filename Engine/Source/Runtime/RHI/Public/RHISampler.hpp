#pragma once
#include "RHIResource.hpp"

namespace worse
{

    class RHISampler : public RHIResource
    {
        void nativeCreate();

    public:
        RHISampler(
            RHIFilter filterMin               = RHIFilter::Linear,
            RHIFilter filterMag               = RHIFilter::Linear,
            RHIFilter filterMipmap            = RHIFilter::Linear,
            RHISamplerAddressMode addressMode = RHISamplerAddressMode::Wrap,
            RHICompareOperation compareOp     = RHICompareOperation::Always,
            bool enableCompare = false, float anisotroy = 0.0f,
            float mipLodBias = 0.0f);
        ~RHISampler();

        // clang-format off
        RHIFilter             getFilterMin() const     { return m_filterMin; }
        RHIFilter             getFilterMag() const     { return m_filterMag; }
        RHIFilter             getFilterMipmap() const  { return m_filterMipmap; }
        RHISamplerAddressMode getAddressMode() const   { return m_addressMode; }
        RHICompareOperation   getCompareOp() const     { return m_compareOp; }
        bool                  isCompareEnabled() const { return m_enableCompare; }
        float                 getAnisotroy() const     { return m_anisotroy; }
        RHINativeHandle       getHandle() const        { return m_handle; }
        // clang-format on

    private:
        RHIFilter m_filterMin;
        RHIFilter m_filterMag;
        RHIFilter m_filterMipmap;
        RHISamplerAddressMode m_addressMode;

        // for SamplerComprisonState
        RHICompareOperation m_compareOp;
        bool m_enableCompare;

        float m_anisotroy;
        float m_mipLodBias;

        RHINativeHandle m_handle = {};
    };

} // namespace worse