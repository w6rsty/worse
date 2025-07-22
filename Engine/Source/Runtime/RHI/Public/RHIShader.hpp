#pragma once
#include "RHIResource.hpp"
#include "RHIVertex.hpp"
#include "RHIDescriptor.hpp"

#include <regex>
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_set>

namespace worse
{
    enum class RHIShaderCompilationState
    {
        Idle,
        Compiling,
        CompiledSuccess,
        CompiledFailure,
    };

    class PreprocessIncludesParser
    {
        std::string recursiveParse(std::filesystem::path const& path);

    public:
        std::string parse(std::filesystem::path const& path);

    private:
        std::unordered_set<std::filesystem::path> m_includes;

        std::regex const includeRegex{R"(^\s*#include\s+\"([^"]+)\")"};
    };

    class RHIShader : public RHIResource
    {
        RHINativeHandle nativeCompile();
        // extract descriptor from spirv
        void reflect(RHIShaderType const shaderType, u32* spirvData,
                     usize const spirvSize);

    public:
        RHIShader(std::string_view name);
        ~RHIShader();

        void compile(std::filesystem::path const& filepath,
                     RHIShaderType const shaderType,
                     RHIVertexType const vertexType = RHIVertexType::None);

        // clang-format off
        std::string_view                  getEntryPoint() const;
        std::vector<RHIDescriptor> const& getDescriptors() const { return m_descriptors; }
        RHIShaderCompilationState         getState() const       { return m_state; }
        RHIShaderType                     getShaderType() const  { return m_shaderType; }
        RHIVertexType                     getVertexType() const  { return m_vertexType; }
        RHIInputLayout const&             getInputLayout() const { return m_inputLayout; }
        u64                     getHash() const        { return m_hash; }
        RHINativeHandle                   getHandle() const      { return m_shaderModule; }
        // clang-format on

    private:
        inline static PreprocessIncludesParser preprocessParser;

        std::filesystem::path m_path;
        std::string m_source;

        std::vector<RHIDescriptor> m_descriptors;
        RHIShaderCompilationState m_state = RHIShaderCompilationState::Idle;
        RHIShaderType m_shaderType        = RHIShaderType::Max;
        RHIVertexType m_vertexType        = RHIVertexType::None;
        RHIInputLayout m_inputLayout;

        u64 m_hash                     = 0;
        RHINativeHandle m_shaderModule = {};
    };

} // namespace worse