#include "Log.hpp"
#include "FileStream.hpp"
#include "Definitions.hpp"

namespace worse
{

    FileStream::FileStream(std::filesystem::path const& path,
                           FileStreamUsageFlags usage)
    {
        m_isOpen = false;
        m_usage  = usage;

        std::ios_base::openmode mode = std::ios_base::binary;
        // clang-format off
        if (m_usage & FileStreamUsageFlagBits::Read) { mode |= std::ios_base::in; }
        if (m_usage & FileStreamUsageFlagBits::Write) { mode |= std::ios_base::out; }
        // clang-format on

        if (m_usage & FileStreamUsageFlagBits::Read)
        {
            m_stream.open(path, mode);
            if (m_stream.fail())
            {
                WS_LOG_ERROR("FileStream",
                             "Failed to open {} for reading",
                             path.string());
                return;
            }
        }

        if (m_usage & FileStreamUsageFlagBits::Write)
        {
            m_stream.open(path, mode);
            if (m_stream.fail())
            {
                WS_LOG_ERROR("FileStream",
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

        if ((m_usage & FileStreamUsageFlagBits::Read))
        {
            m_stream.clear();
            m_stream.close();
        }

        if ((m_usage & FileStreamUsageFlagBits::Write))
        {
            m_stream.flush();
            m_stream.close();
        }

        m_isOpen = false;
    }

    std::string FileStream::read()
    {
        WS_ASSERT(m_isOpen);
        WS_ASSERT(m_usage & FileStreamUsageFlagBits::Read);

        std::string content((std::istreambuf_iterator<char>(m_stream)),
                            std::istreambuf_iterator<char>());
        return content;
    }

    void FileStream::write()
    {
        UNIMPLEMENTED();
    }

} // namespace worse