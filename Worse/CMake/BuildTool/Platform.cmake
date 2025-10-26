include_guard()

if(MSVC)
    add_compile_options(/Zc:preprocessor)
endif()

if(WIN32)
    add_compile_definitions(WS_PLATFORM_WINDOWS)
elseif(APPLE)
    add_compile_definitions(WS_PLATFORM_APPLE)
endif()