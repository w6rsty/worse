#include "FileSystem.hpp"

#include <fstream>

namespace worse
{

    std::string FileSystem::readFile(std::string const& filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            return {};
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        file.close();
        return content;
    }

} // namespace worse