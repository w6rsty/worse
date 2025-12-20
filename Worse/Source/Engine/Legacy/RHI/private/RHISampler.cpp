#include "RHISampler.hpp"
#include "RHIDevice.hpp"

namespace Worse
{
    RHISampler::RHISampler(RHIFilter filterMin, RHIFilter filterMag,
                           RHIFilter filterMipmap,
                           RHISamplerAddressMode addressMode,
                           RHICompareOperation compareOp, Bool enableCompare,
                           Float anisotroy, Float mipLodBias)
    {
        m_filterMin     = filterMin;
        m_filterMag     = filterMag;
        m_filterMipmap  = filterMipmap;
        m_addressMode   = addressMode;
        m_compareOp     = compareOp;
        m_enableCompare = enableCompare;
        m_anisotroy     = anisotroy;
        m_mipLodBias    = mipLodBias;

        nativeCreate();
    }
    RHISampler::~RHISampler()
    {
        RHIDevice::deletionQueueAdd(m_handle);
    }
} // namespace Worse