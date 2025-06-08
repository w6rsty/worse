#pragma once
#include "dxc/dxcapi.h"

#include <string>

namespace worse
{

    class DXCompiler
    {
    public:
        static void initialize();
        static void shutdown();
        static DXCompiler* instance();

        DXCompiler();
        ~DXCompiler();

        CComPtr<IDxcBlob> compile(std::string const& source,
                                  std::vector<std::wstring> const& wArguments);

    private:
        static inline DXCompiler* s_instance = nullptr;

        CComPtr<IDxcCompiler3> m_compiler;
        CComPtr<IDxcUtils> m_utils;
    };

} // namespace worse