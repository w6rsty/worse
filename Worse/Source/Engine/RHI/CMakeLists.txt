include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(RHI)

    PrivateDependencyModuleAdd(Common)
    PrivateDependencyModuleAdd(Core)

    PrivateDependencyModuleAdd(Vulkan::Headers)

EndDeclareModule()
