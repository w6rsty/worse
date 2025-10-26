include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(RHI)

    PrivateDependencyModuleAdd(GPUOpen::VulkanMemoryAllocator)
    PrivateDependencyModuleAdd(Vulkan-Headers)
    PrivateDependencyModuleAdd(DXC)
    PrivateDependencyModuleAdd(spirv-reflect-static)
    PrivateDependencyModuleAdd(imgui)

    PublicDependencyModuleAdd(volk_headers)
    PublicDependencyModuleAdd(Core)
    PublicDependencyModuleAdd(FileSystem)
    PublicDependencyModuleAdd(Asset)

EndDeclareModule()
