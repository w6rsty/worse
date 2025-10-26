#pragma once
#include <filesystem>

namespace worse
{

    class FileSystem
    {
    public:
        static bool isSupportedImage(std::filesystem::path const& path);

        // Checks if the given path, file or directory, exists
        static bool isPathExists(std::filesystem::path const& path);
        // Checks if the given path is a existing file
        static bool isFileExists(std::filesystem::path const& path);
        // Checks if the given path is a existing directory
        static bool isDirectoryExists(std::filesystem::path const& path);
    };

} // namespace worse