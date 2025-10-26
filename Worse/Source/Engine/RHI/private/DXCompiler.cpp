#include "DXCompiler.hpp"
#include "Log.hpp"

namespace worse
{

    void DXCompiler::initialize()
    {
        if (!s_instance)
        {
            s_instance = new DXCompiler();
        }
    }

    void DXCompiler::shutdown()
    {
        if (s_instance)
        {
            delete s_instance;
            s_instance = nullptr;
        }
    }

    DXCompiler* DXCompiler::instance()
    {
        return s_instance;
    }

    DXCompiler::DXCompiler()
    {
        HRESULT hres;

        hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));
        if (FAILED(hres))
        {
            WS_LOG_ERROR("dxc", "Failed to initialize DXC compiler");
        }

        hres = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils));
        if (FAILED(hres))
        {
            WS_LOG_ERROR("dxc", "Failed to initialize DXC utils");
        }
    }

    DXCompiler::~DXCompiler()
    {
        if (m_utils)
        {
            m_utils.Release();
        }
        if (m_compiler)
        {
            m_compiler.Release();
        }
    }

    CComPtr<IDxcBlob> DXCompiler::compile(std::string const& source, std::vector<std::wstring> const& wArguments)
    {
        CComPtr<IDxcBlobEncoding> sourceBlob;
        if (FAILED(m_utils->CreateBlobFromPinned(source.data(), static_cast<u32>(source.size()), CP_UTF8, &sourceBlob)))
        {
            WS_LOG_ERROR("dxc", "Failed to load shader file");
        }

        DxcBuffer buffer = {};
        buffer.Ptr       = sourceBlob->GetBufferPointer();
        buffer.Size      = sourceBlob->GetBufferSize();
        buffer.Encoding  = DXC_CP_ACP;

        std::vector<LPCWSTR> wArgumentCPtrs;
        wArgumentCPtrs.reserve(wArguments.size());
        for (std::wstring const& arg : wArguments)
        {
            wArgumentCPtrs.push_back(arg.c_str());
        }

        CComPtr<IDxcResult> dxcResult = nullptr;

        HRESULT result = m_compiler->Compile(&buffer, wArgumentCPtrs.data(), static_cast<u32>(wArgumentCPtrs.size()), nullptr, IID_PPV_ARGS(&dxcResult));

        if (SUCCEEDED(result))
        {
            dxcResult->GetStatus(&result);
        }

        if (FAILED(result))
        {
            if (dxcResult)
            {
                CComPtr<IDxcBlobEncoding> errorBlob = nullptr;
                HRESULT res = dxcResult->GetErrorBuffer(&errorBlob);
                bool suc = SUCCEEDED(res);
                if (suc && errorBlob)
                {
                    WS_LOG_ERROR("dxc", "{}", static_cast<char const*>(errorBlob->GetBufferPointer()));
                }
            }
            return nullptr;
        }

        CComPtr<IDxcBlob> codeBlob = nullptr;
        if (dxcResult)
        {
            dxcResult->GetResult(&codeBlob);
        }

        return codeBlob;
    }

} // namespace worse