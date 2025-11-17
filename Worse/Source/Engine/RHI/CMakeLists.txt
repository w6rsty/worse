include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(RHI)

    Deprecated()

    PrivateDependencyModuleAdd(Common)
    PrivateDependencyModuleAdd(Core)
    PrivateDependencyModuleAdd(FileSystem)
    PrivateDependencyModuleAdd(Asset)

    PrivateDependencyModuleAdd(Vulkan-Headers)
    PrivateDependencyModuleAdd(VulkanMemoryAllocator)
    PrivateDependencyModuleAdd(DXC)
    PrivateDependencyModuleAdd(spirv-reflect-static)
    PrivateDependencyModuleAdd(imgui)

    PublicDependencyModuleAdd(volk_headers)

EndDeclareModule()
