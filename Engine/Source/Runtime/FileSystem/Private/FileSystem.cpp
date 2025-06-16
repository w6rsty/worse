#include "FileSystem.hpp"

#include <array>
#include <string>

namespace worse
{
    namespace
    {
        // clang-format off
        constexpr std::array<std::string, 3> supporteImagedExtensions = {
            ".png",
            ".jpg",
            // ".dds",
        };
        // clang-format on
    } // namespace

    bool FileSystem::isSupportedImage(std::filesystem::path const& path)
    {
        if (!path.has_extension())
        {
            return false;
        }

        return std::find(supporteImagedExtensions.begin(),
                         supporteImagedExtensions.end(),
                         path.extension().string()) !=
               supporteImagedExtensions.end();
    }

    bool isPathExists(std::filesystem::path const& path)
    {
        return std::filesystem::exists(path);
    }

    bool isFileExists(std::filesystem::path const& path)
    {
        return std::filesystem::is_regular_file(path);
    }

    bool isDirectoryExists(std::filesystem::path const& path)
    {
        return std::filesystem::is_directory(path);
    }

} // namespace worse