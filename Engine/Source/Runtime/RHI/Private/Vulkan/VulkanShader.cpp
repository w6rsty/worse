#include "DXCompiler.hpp"
#include "RHIDevice.hpp"
#include "RHIShader.hpp"

#include "spirv_cross/spirv_hlsl.hpp"

namespace worse
{
    namespace
    {
        void
        spirvExtractDescriptor(spirv_cross::CompilerHLSL const& hlsl,
                               spirv_cross::ShaderResources shaderResources,
                               RHIDescriptorType const descriptorType,
                               RHIShaderStageFlags const shaderStage,
                               std::vector<RHIDescriptor>& descriptors)
        {
            spirv_cross::SmallVector<spirv_cross::Resource> resources;
            switch (descriptorType)
            {
                // clang-format off
            case RHIDescriptorType::Image:              resources = shaderResources.separate_images;       break;
            case RHIDescriptorType::TextureStorage:     resources = shaderResources.storage_images;        break;
            case RHIDescriptorType::PushConstantBuffer: resources = shaderResources.push_constant_buffers; break;
            case RHIDescriptorType::ConstantBuffer:     resources = shaderResources.uniform_buffers;       break;
            case RHIDescriptorType::StructuredBuffer:   resources = shaderResources.storage_buffers;       break;
            default:                                                                                       break;
                // clang-format on
            }

            RHIImageLayout layout = RHIImageLayout::Undefined;
            layout = descriptorType == RHIDescriptorType::TextureStorage
                         ? RHIImageLayout::General
                         : layout;
            layout = descriptorType == RHIDescriptorType::Image
                         ? RHIImageLayout::ShaderRead
                         : layout;

            for (spirv_cross::Resource const& resource : resources)
            {
                // clang-format off
                RHIDescriptor descriptor = {};
                descriptor.name          = resource.name;
                descriptor.slot          = hlsl.get_decoration(resource.id, spv::DecorationBinding);
                descriptor.stageFlags    = shaderStage;
                descriptor.type          = descriptorType;
                descriptor.layout        = layout;

                auto spirvType = hlsl.get_type(resource.type_id);
                if ((descriptorType == RHIDescriptorType::PushConstantBuffer) ||
                    (descriptorType == RHIDescriptorType::ConstantBuffer))
                {
                    descriptor.size = hlsl.get_declared_struct_size(spirvType);
                }
                descriptor.isArray     = spirvType.array.size() > 0;
                descriptor.arrayLength = descriptor.isArray ? spirvType.array[0] : 0;
                // clang-format on

                descriptors.push_back(descriptor);
            }
        }
    } // namespace

    RHINativeHandle RHIShader::nativeCompile()
    {
        std::vector<std::wstring> wArguments;
        switch (m_shaderType)
        {
        case RHIShaderType::Vertex:
            wArguments = {L"-T", L"vs_6_8", L"-E", L"main_vs"};
            break;
        case RHIShaderType::Pixel:
            wArguments = {L"-T", L"ps_6_8", L"-E", L"main_ps"};
            break;
        case RHIShaderType::Compute:
            wArguments = {L"-T", L"cs_6_8", L"-E", L"main_cs"};
            break;
        default:
            WS_ASSERT(false);
            break;
        }

        wArguments.push_back(L"-spirv");
        std::string filepath   = m_path.string();
        std::wstring wFilepath = std::wstring(filepath.begin(), filepath.end());
        wArguments.push_back(wFilepath);

        RHINativeHandle handle = {};

        CComPtr<IDxcBlob> codeBlob =
            DXCompiler::instance()->compile(m_source, wArguments);

        if (!codeBlob)
        {
            WS_LOG_ERROR("Shader", "Compilation failed");
            return handle;
        }

        // clang-format off
        VkShaderModuleCreateInfo infoShader = {};
        infoShader.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        infoShader.pCode                    = static_cast<std::uint32_t*>(codeBlob->GetBufferPointer());
        infoShader.codeSize                 = codeBlob->GetBufferSize();
        // clang-format on

        VkShaderModule shader = VK_NULL_HANDLE;
        WS_ASSERT_VK(vkCreateShaderModule(RHIContext::device,
                                          &infoShader,
                                          nullptr,
                                          &shader));
        handle = RHINativeHandle{shader, RHINativeHandleType::Shader};
        RHIDevice::setResourceName(handle, m_name);

        reflect(m_shaderType,
                static_cast<std::uint32_t*>(codeBlob->GetBufferPointer()),
                codeBlob->GetBufferSize() / sizeof(std::uint32_t));

        codeBlob.Release();

        return handle;
    }

    void RHIShader::reflect(RHIShaderType const shaderType,
                            std::uint32_t* spirvData,
                            std::size_t const spirvSize)
    {
        WS_ASSERT(spirvData != nullptr);
        WS_ASSERT(spirvSize > 0);

        spirv_cross::CompilerHLSL hlsl{spirvData, spirvSize};
        spirv_cross::ShaderResources resources = hlsl.get_shader_resources();

        // clang-format off
        RHIShaderStageFlags shaderStage = rhiShaderStageFlags(shaderType);
        spirvExtractDescriptor(hlsl, resources, RHIDescriptorType::Image,              shaderStage, m_descriptors);
        spirvExtractDescriptor(hlsl, resources, RHIDescriptorType::TextureStorage,     shaderStage, m_descriptors);
        spirvExtractDescriptor(hlsl, resources, RHIDescriptorType::PushConstantBuffer, shaderStage, m_descriptors);
        spirvExtractDescriptor(hlsl, resources, RHIDescriptorType::ConstantBuffer,     shaderStage, m_descriptors);
        spirvExtractDescriptor(hlsl, resources, RHIDescriptorType::StructuredBuffer,   shaderStage, m_descriptors);
        // clang-format on

        for (auto& descriptor : m_descriptors)
        {
            std::string arr = "";
            if (descriptor.isArray)
            {
                arr = "[" + std::to_string(descriptor.arrayLength) + "]";
            }
            WS_LOG_DEBUG("spirv-cross",
                         "{}{} - {} ",
                         rhiDescriptorTypeToString(descriptor.type),
                         arr,
                         descriptor.name);
        }
    }

} // namespace worse