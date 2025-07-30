add_executable(Example00 Example00.cpp)
target_compile_features(Example00 PRIVATE cxx_std_20)
target_link_libraries(Example00 PRIVATE
    Worse::Engine
    Worse::Renderer
    Worse::ECS
)

# Post-build step to copy required DLLs
if(WIN32)
    # Find SDL3 DLL
    if(TARGET SDL3::SDL3)
        get_target_property(SDL3_DLL_PATH SDL3::SDL3 IMPORTED_LOCATION)
        if(NOT SDL3_DLL_PATH)
            get_target_property(SDL3_DLL_PATH SDL3::SDL3 IMPORTED_LOCATION_DEBUG)
        endif()
        if(NOT SDL3_DLL_PATH)
            get_target_property(SDL3_DLL_PATH SDL3::SDL3 IMPORTED_LOCATION_RELEASE)
        endif()
    endif()

    # Add post-build command to copy SDL3.dll
    if(SDL3_DLL_PATH AND EXISTS ${SDL3_DLL_PATH})
        message(STATUS "Found SDL3 DLL at: ${SDL3_DLL_PATH}")
        add_custom_command(TARGET Example00 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${SDL3_DLL_PATH}
                $<TARGET_FILE_DIR:Example00>
            COMMENT "Copying SDL3.dll to output directory"
        )
    else()
        # Fallback: try to find SDL3.dll in common locations
        find_file(SDL3_DLL_FALLBACK
            NAMES SDL3.dll
            PATHS
                ${CMAKE_BINARY_DIR}/_deps/sdl3-build
                ${CMAKE_BINARY_DIR}/_deps/sdl3-build/Debug
                ${CMAKE_BINARY_DIR}/_deps/sdl3-build/Release
                ${CMAKE_BINARY_DIR}/_deps/sdl3-build/$<CONFIG>
            NO_DEFAULT_PATH
        )
        if(SDL3_DLL_FALLBACK)
            message(STATUS "Found SDL3 DLL (fallback) at: ${SDL3_DLL_FALLBACK}")
            add_custom_command(TARGET Example00 POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${SDL3_DLL_FALLBACK}
                    $<TARGET_FILE_DIR:Example00>
                COMMENT "Copying SDL3.dll to output directory (fallback)"
            )
        else()
            message(WARNING "Could not find SDL3.dll in any expected locations")
        endif()
    endif()

    # Copy AddressSanitizer runtime DLL for debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_CONFIGURATION_TYPES)
        # Try to find the ASAN runtime DLL
        find_file(ASAN_DLL
            NAMES clang_rt.asan_dynamic-x86_64.dll
            PATHS
                # Visual Studio paths
                "$ENV{VCINSTALLDIR}/Tools/MSVC/$ENV{VCToolsVersion}/bin/Hostx64/x64"
                "$ENV{VCToolsInstallDir}/bin/Hostx64/x64"
                # Windows SDK paths
                "$ENV{WindowsSdkDir}/bin/$ENV{WindowsSDKVersion}/x64"
                "$ENV{WindowsSdkDir}/bin/x64"
                # LLVM/Clang paths
                "$ENV{PROGRAMFILES}/LLVM/bin"
                "$ENV{ProgramFiles\(x86\)}/LLVM/bin"
                # CMake toolchain paths
                "${CMAKE_C_COMPILER}/../"
                "${CMAKE_CXX_COMPILER}/../"
            NO_DEFAULT_PATH
        )
        
        if(ASAN_DLL)
            message(STATUS "Found AddressSanitizer DLL at: ${ASAN_DLL}")
            add_custom_command(TARGET Example00 POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${ASAN_DLL}
                    $<TARGET_FILE_DIR:Example00>
                COMMENT "Copying AddressSanitizer runtime DLL to output directory"
            )
        else()
            message(WARNING "Could not find clang_rt.asan_dynamic-x86_64.dll. AddressSanitizer may not work properly.")
        endif()
    endif()
endif()

