#include "FileStream.hpp"
#include "Math/Hash.hpp"
#include "Profiling/Stopwatch.hpp"
#include "RHIDevice.hpp"
#include "RHIShader.hpp"

#include <functional>

namespace worse
{

    RHIShader::RHIShader(std::string_view name) : RHIResource(name)
    {
    }

    RHIShader::~RHIShader()
    {
        // TODO: Shader lifecycle is special
        //     if (m_shaderModule)
        //     {
        //         RHIDevice::deletionQueueAdd(m_shaderModule);
        //         m_shaderModule = {};
        //     }
    }

    void RHIShader::compile(std::filesystem::path const& filepath,
                            RHIShaderType const shaderType,
                            RHIVertexType const vertexType)
    {
        m_source.clear();
        m_descriptors.clear();
        m_inputLayout = {};

        m_path        = filepath;
        m_shaderType  = shaderType;
        m_vertexType  = vertexType;
        m_inputLayout = RHIInputLayout(m_vertexType);

        FileStream fileStream(filepath, FileStreamUsageFlagBits::Read);
        if (!fileStream.isOpen())
        {
            WS_LOG_ERROR("rhi",
                         "Failed to read shader file: {}",
                         filepath.string());
            return;
        }
        m_source = fileStream.read();
        fileStream.close();

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
                WS_LOG_INFO("Shader",
                            "Compiled: {} took: {:.1f}ms",
                            m_path.string(),
                            sw.elapsedMs());
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