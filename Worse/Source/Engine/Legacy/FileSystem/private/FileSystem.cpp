#include "FileSystem.hpp"

#include <array>
#include <string>

namespace Worse
{
    namespace
    {
        // clang-format off
        std::array<std::string, 3> supporteImagedExtensions = {
            ".png",
            ".jpg",
        };
        // clang-format on
    } // namespace

    Bool FileSystem::isSupportedImage(std::filesystem::path const& path)
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

    Bool FileSystem::isPathExists(std::filesystem::path const& path)
    {
        return std::filesystem::exists(path);
    }

    Bool FileSystem::isFileExists(std::filesystem::path const& path)
    {
        return std::filesystem::is_regular_file(path);
    }

    Bool FileSystem::isDirectoryExists(std::filesystem::path const& path)
    {
        return std::filesystem::is_directory(path);
    }

} // namespace Worse