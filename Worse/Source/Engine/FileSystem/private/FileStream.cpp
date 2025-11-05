#include "common_macro.hpp"
#include "logger/logger.hpp"
#include "FileStream.hpp"

namespace Worse
{

    FileStream::FileStream(std::filesystem::path const& path, FileStreamUsage::Flags usageFlags)
    {
        m_isOpen     = false;
        m_usageFlags = usageFlags;

        std::ios_base::openmode mode = std::ios_base::binary;
        // clang-format off
        if (m_usageFlags & FileStreamUsage::FlagBits::Read) { mode |= std::ios_base::in; }
        if (m_usageFlags & FileStreamUsage::FlagBits::Write) { mode |= std::ios_base::out; }
        // clang-format on

        if (m_usageFlags & FileStreamUsage::FlagBits::Read)
        {
            m_stream.open(path, mode);
            if (m_stream.fail())
            {
                WORSE_LOG_ERROR("FileStream",
                                "Failed to open {} for reading",
                                path.string());
                return;
            }
        }

        if (m_usageFlags & FileStreamUsage::FlagBits::Write)
        {
            m_stream.open(path, mode);
            if (m_stream.fail())
            {
                WORSE_LOG_ERROR("FileStream",
                                "Failed to open {} for writing",
                                path.string());
                return;
            }
        }

        m_isOpen = m_stream.is_open();
    }

    FileStream::~FileStream()
    {
        close();
    }

    void FileStream::close()
    {
        if (!m_isOpen)
        {
            return;
        }

        if ((m_usageFlags & FileStreamUsage::FlagBits::Read))
        {
            m_stream.clear();
            m_stream.close();
        }

        if ((m_usageFlags & FileStreamUsage::FlagBits::Write))
        {
            m_stream.flush();
            m_stream.close();
        }

        m_isOpen = false;
    }

    std::string FileStream::read()
    {
        WORSE_ASSERT(m_isOpen);
        WORSE_ASSERT(m_usageFlags & FileStreamUsage::FlagBits::Read);

        std::string content((std::istreambuf_iterator<char>(m_stream)), std::istreambuf_iterator<char>());
        return content;
    }

    void FileStream::write()
    {
        WORSE_UNIMPLEMENTED();
    }

} // namespace Worse