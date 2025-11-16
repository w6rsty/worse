#include "macro/common_macro.hpp"
#include "math/math.hpp"
#include "memory/memory.hpp"

#include <cstring>

namespace Worse
{

    namespace Common::Detail
    {

        WORSE_NORETURN void OnMallocFailed()
        {
            // TODO: How to abort?
        }

    } // namespace Common::Detail

    /* Malloc using C++ std */
    struct StdMalloc
    {
        struct HeapAllocationHeader
        {
            Size size;
            UInt alignment;
            void* raw;
        };

        static void* Malloc(Size count, UInt alignment)
        {
            if (count == 0) return nullptr;

            alignment = Max(alignment, alignof(HeapAllocationHeader));

            Size const headerSize = sizeof(HeapAllocationHeader);
            Size const totalSize  = count + headerSize + alignment - 1;

            void* raw = ::operator new(totalSize, std::align_val_t(alignof(std::max_align_t)));

            UPtrInt rawAddr      = r_cast<UPtrInt>(raw);
            UPtrInt minHeaderPos = rawAddr + headerSize;
            UPtrInt alignedUser  = AlignUp(minHeaderPos, alignment);

            HeapAllocationHeader* header = r_cast<HeapAllocationHeader*>(alignedUser - headerSize);

            header->size      = count;
            header->alignment = alignment;
            header->raw       = raw;

            return r_cast<void*>(alignedUser);
        }

        static void* Realloc(void* original, Size count, UInt alignment)
        {
            if (!original) return Malloc(count, alignment);
            if (count == 0)
            {
                Free(original);
                return nullptr;
            }

            HeapAllocationHeader* oldHeader = r_cast<HeapAllocationHeader*>(r_cast<UByte*>(original) - sizeof(HeapAllocationHeader));

            Size oldSize  = oldHeader->size;
            UInt oldAlign = oldHeader->alignment;

            if (alignment == 0) alignment = oldAlign;

            void* newPtr = Malloc(count, alignment);

            Size copySize = Min(oldSize, count);
            std::memcpy(newPtr, original, copySize);

            Free(original);
            return newPtr;
        }

        static void Free(void* userPtr)
        {
            if (!userPtr) return;

            auto header = r_cast<HeapAllocationHeader*>(r_cast<UByte*>(userPtr) - sizeof(HeapAllocationHeader));

            void* raw = header->raw;
            ::operator delete(raw, std::align_val_t(alignof(std::max_align_t)));
        }

    private:
        static WORSE_FORCE_INLINE UPtrInt AlignUp(UPtrInt ptrInt, UPtrInt alignment)
        {
            return (ptrInt + alignment - 1) & ~(alignment - 1);
        }
    };

    /* Memory operation interface */
    struct MallocFunction
    {
        void* (*Malloc)(Size count, UInt alignment);
        void* (*Realloc)(void* orignal, Size count, UInt aligment);
        void (*Free)(void* original);
    };

    /* Support runtime switching */
    static MallocFunction gMallocFunction = MallocFunction{
        &StdMalloc::Malloc,
        &StdMalloc::Realloc,
        &StdMalloc::Free,
    };

    void* Memory::Malloc(Size count, UInt alignment)
    {
        return gMallocFunction.Malloc(count, alignment);
    }

    void* Memory::Realloc(void* original, Size count, UInt alignment)
    {
        return gMallocFunction.Realloc(original, count, alignment);
    }

    void Memory::Free(void* original)
    {
        gMallocFunction.Free(original);
    }

} // namespace Worse