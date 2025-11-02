#pragma once
#include "base_type.hpp"
#include "bit_flag.hpp"

#include <fstream>
#include <filesystem>

namespace Worse
{
    WORSE_BEGIN_DECLARE_BIT_FLAG(FileStreamUsage, UInt)
    // clang-format off
    WORSE_DECLARE_FLAG_BIT(Unknown, 0);
    WORSE_DECLARE_FLAG_BIT(Read,    1 << 1);
    WORSE_DECLARE_FLAG_BIT(Write,   1 << 2);
    // clang-format on
    WORSE_END_DECLARE_BIT_FLAG(FileStreamUsage)

    //WS_DEFINE_FLAGS(FileStreamUsage, UInt);
    //struct FileStreamUsageFlagBits
    //{
    //    static constexpr FileStreamUsageFlags Read{1 << 0};
    //    static constexpr FileStreamUsageFlags Write{1 << 1};
    //};

    class FileStream
    {
    public:
        FileStream(std::filesystem::path const& path, FileStreamUsage::Flags usageFlags);
        ~FileStream();

        void close();

        // read string file
        std::string read();

        void write();

        // clang-format off
        Bool isOpen() const { return m_isOpen; }
        // clang-format on

    private:
        std::fstream m_stream;

        Bool m_isOpen;
        FileStreamUsage::Flags m_usageFlags;
    };

}; // namespace Worse