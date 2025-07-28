#include "Math/Hash.hpp"
#include "Profiling/Stopwatch.hpp"
#include "FileSystem.hpp"
#include "RHIDevice.hpp"
#include "RHIShader.hpp"

#include <fstream>
#include <sstream>
#include <functional>

namespace worse
{
    std::string
    PreprocessIncludesParser::recursiveParse(std::filesystem::path const& path)
    {
        std::filesystem::path canonicalPath;
        try
        {
            canonicalPath = std::filesystem::canonical(path);
        }
        catch (std::filesystem::filesystem_error const& e)
        {
            WS_LOG_ERROR("Shader", "Failed to canonicalize path: {}", e.what());
            return {};
        }

        // skip duplicate includes
        if (m_includes.count(canonicalPath))
        {
            return {};
        }

        m_includes.insert(canonicalPath);

        std::ifstream fileStream(canonicalPath);
        if (!fileStream.is_open())
        {
            WS_LOG_ERROR("Shader",
                         "Failed to open file: {}",
                         canonicalPath.string());
            return {};
        }

        std::stringstream outputStream;
        std::string line;
        int lineNumber = 0;

        std::filesystem::path baseDir = canonicalPath.parent_path();

        while (std::getline(fileStream, line))
        {
            ++lineNumber;
            std::smatch match;
            if (std::regex_match(line, match, includeRegex))
            {
                std::string includeName = match[1].str();

                std::filesystem::path includePath = baseDir / includeName;

                if (!FileSystem::isFileExists(includePath))
                {
                    WS_LOG_ERROR("Shader", "{} (line {}) does not exist", includePath.string(), lineNumber);
                    continue;
                }
                else
                {
                    outputStream << recursiveParse(includePath);
                }
            }
            else
            {
                outputStream << line << '\n';
            }
        }

        fileStream.close();
        return outputStream.str();
    }

    std::string PreprocessIncludesParser::parse(std::filesystem::path const& path)
    {
        m_includes.clear();

        if (!FileSystem::isFileExists(path))
        {
            WS_LOG_ERROR("Shader", "Failed to read file: {}", path.string());
            return {};
        }

        return recursiveParse(path);
    }

    RHIShader::RHIShader(std::string_view name) : RHIResource(name)
    {
    }

    RHIShader::~RHIShader()
    {
    }

    void RHIShader::compile(std::filesystem::path const& filepath, RHIShaderType const shaderType, RHIVertexType const vertexType)
    {
        m_source.clear();
        m_descriptors.clear();
        m_inputLayout = {};

        m_path        = filepath;
        m_shaderType  = shaderType;
        m_vertexType  = vertexType;
        m_inputLayout = RHIInputLayout(m_vertexType);

        m_source = preprocessParser.parse(m_path);

        // generate hash
        {
            std::hash<std::string> hasher;
            m_hash = math::hashCombine(m_hash, hasher(m_path));

            // TODO: hash definitions
        }

        // compile
        {
            profiling::Stopwatch sw;
            m_state        = RHIShaderCompilationState::Compiling;
            m_shaderModule = nativeCompile();
            RHIDevice::deletionQueueAdd(m_shaderModule);
            m_state = m_shaderModule
                          ? RHIShaderCompilationState::CompiledSuccess
                          : RHIShaderCompilationState::CompiledFailure;

            if (m_state == RHIShaderCompilationState::CompiledSuccess)
            {
                WS_LOG_INFO("Shader", "Compiled: {} took: {:.1f}ms", m_path.string(), sw.elapsedMs());
            }
        }
    }

    std::string_view RHIShader::getEntryPoint() const
    {
        switch (m_shaderType)
        {
            // clang-format off
        case RHIShaderType::Vertex:  return "main_vs";
        case RHIShaderType::Pixel:   return "main_ps";
        case RHIShaderType::Compute: return "main_cs";
        default:                     return "";
            // clang-format on
        }
    }

} // namespace worse