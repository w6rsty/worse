#pragma once
#include "RHIResource.hpp"
#include "RHIDescriptor.hpp"

#include <string>
#include <filesystem>
#include <vector>

namespace worse
{
    enum class RHIShaderCompilationState
    {
        Idle,
        Compiling,
        CompiledSuccess,
        CompiledFailure,
    };

    class RHIShader : public RHIResource
    {
        RHINativeHandle nativeCompile();
        // extract descriptor from spirv
        void reflect(RHIShaderType const shaderType, std::uint32_t* spirvData,
                     std::size_t const spirvSize);

    public:
        RHIShader(std::string_view name);
        ~RHIShader();

        void compile(std::filesystem::path const& filepath,
                     RHIShaderType const shaderType);

        // clang-format off
        std::string_view getEntryPoint() const;
        std::vector<RHIDescriptor> const& getDescriptors() const { return m_descriptors; }
        RHIShaderCompilationState getState() const               { return m_state; }
        RHIShaderType getShaderType() const                      { return m_shaderType; }
        RHINativeHandle getHandle() const                        { return m_shaderModule; }
        // clang-format on

    private:
        std::filesystem::path m_path;
        std::string m_source;

        std::vector<RHIDescriptor> m_descriptors;
        RHIShaderCompilationState m_state = RHIShaderCompilationState::Idle;
        RHIShaderType m_shaderType        = RHIShaderType::Max;
        RHINativeHandle m_shaderModule    = {};
    };

} // namespace worse