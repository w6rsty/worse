#pragma once
#include "base_type.hpp"

#include <filesystem>

namespace Worse
{

    class FileSystem
    {
    public:
        static Bool isSupportedImage(std::filesystem::path const& path);

        // Checks if the given path, file or directory, exists
        static Bool isPathExists(std::filesystem::path const& path);
        // Checks if the given path is a existing file
        static Bool isFileExists(std::filesystem::path const& path);
        // Checks if the given path is a existing directory
        static Bool isDirectoryExists(std::filesystem::path const& path);
    };

} // namespace Worse