#pragma once
#include "base_type.hpp"
#include "bit_flag.hpp"
#include "math/math.hpp"
#include "RHIResource.hpp"

namespace Worse
{

    WORSE_BEGIN_DECLARE_BIT_FLAG(RHIBufferUsage, UInt)
    // clang-format off
    WORSE_DECLARE_FLAG_BIT(Unknown,  0)
    WORSE_DECLARE_FLAG_BIT(Vertex,   0b0001'0001)
    WORSE_DECLARE_FLAG_BIT(Instance, 0b0001'0010)
    WORSE_DECLARE_FLAG_BIT(Index,    0b0001'0100)
    WORSE_DECLARE_FLAG_BIT(Storage,  0b0010'0000)
    WORSE_DECLARE_FLAG_BIT(Uniform,  0b1000'0000)
    // clang-format on
    WORSE_END_DECLARE_BIT_FLAG(RHIBufferUsage)

    static constexpr UByte VII_BIT       = 0b0001'0000;
    static constexpr UByte VII_MASK      = 0b0000'1111;
    static constexpr UByte VII_ONLT_MASK = 0b0011'1111;

    class RHIBuffer : public RHIResource
    {
        void nativeCreate(void const* data);
        void nativeDestroy();

    public:
        RHIBuffer() = default;
        RHIBuffer(RHIBufferUsage::Flags const usageFlags, UInt const stride,
                  UInt const elementCount, void const* data,
                  Bool const mappable = false, std::string_view name = "Buffer")
        {
            m_usageFlags   = usageFlags;
            m_stride       = stride;
            m_elementCount = elementCount;
            m_size         = m_stride * m_elementCount;
            m_mappable     = mappable;

            nativeCreate(data);
        }

        ~RHIBuffer()
        {
            nativeDestroy();
        }

        // update mapped buffer data
        void update(RHICommandList* cmdList, void const* cpuData, UInt const size);

        void resetOffset()
        {
            m_offset      = 0;
            m_firstUpdate = true;
        }

        // clang-format off
        RHIBufferUsage::Flags getUsageFlags() const  { return m_usageFlags; }
        UInt                getStride() const        { return m_stride; }
        UInt                getOffset() const        { return m_offset; }
        UInt                getElementCount() const  { return m_elementCount; }
        UInt                getSize() const          { return m_size; }
        void*               getMappedData() const    { return m_gpuData; }
        RHINativeHandle     getHandle() const        { return m_handle; }
        // clang-format on
    private:
        RHIBufferUsage::Flags m_usageFlags = RHIBufferUsage::FlagBits::Unknown;
        UInt m_stride                      = 0;
        UInt m_offset                      = 0;
        UInt m_elementCount                = 0;
        UInt m_size                        = 0;
        void* m_gpuData                    = nullptr;
        Bool m_mappable                    = false;
        Bool m_firstUpdate                 = true;

        RHINativeHandle m_handle = {};
    };

} // namespace Worse