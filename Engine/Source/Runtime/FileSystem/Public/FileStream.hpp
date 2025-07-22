#pragma once
#include "Types.hpp"

#include <fstream>
#include <filesystem>

namespace worse
{

    WS_DEFINE_FLAGS(FileStreamUsage, u32);
    struct FileStreamUsageFlagBits
    {
        static constexpr FileStreamUsageFlags Read{1 << 0};
        static constexpr FileStreamUsageFlags Write{1 << 1};
    };

    class FileStream
    {
    public:
        FileStream(std::filesystem::path const& path,
                   FileStreamUsageFlags usage);
        ~FileStream();

        void close();

        // read string file
        std::string read();

        void write();

        // clang-format off
        bool isOpen() const { return m_isOpen; }
        // clang-format on

    private:
        std::fstream m_stream;

        bool m_isOpen;
        FileStreamUsageFlags m_usage;
    };

}; // namespace worse