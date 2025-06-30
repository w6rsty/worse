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

    bool FileSystem::isPathExists(std::filesystem::path const& path)
    {
        return std::filesystem::exists(path);
    }

    bool FileSystem::isFileExists(std::filesystem::path const& path)
    {
        return std::filesystem::is_regular_file(path);
    }

    bool FileSystem::isDirectoryExists(std::filesystem::path const& path)
    {
        return std::filesystem::is_directory(path);
    }

} // namespace worse