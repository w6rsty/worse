#pragma once

// Include C++ standard headers first
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX  // Prevent Windows from defining min/max macros
// Prevent Windows from defining byte typedef that conflicts with std::byte
#define byte windows_byte_override
#include <windows.h>
#include <unknwn.h>
#include <atlbase.h>
#undef byte
// Also undefine Windows near/far macros that conflict with function parameters
#undef near
#undef far
#endif

#include "dxc/dxcapi.h"

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

        CComPtr<IDxcBlob> compile(std::string const& source, std::vector<std::wstring> const& wArguments);

    private:
        static inline DXCompiler* s_instance = nullptr;

        CComPtr<IDxcCompiler3> m_compiler;
        CComPtr<IDxcUtils> m_utils;
    };

} // namespace worse