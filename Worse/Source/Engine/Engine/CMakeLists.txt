include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(Engine)

    PrivateDependencyModuleAdd(Renderer)
    PrivateDependencyModuleAdd(RHI)

    PublicDependencyModuleAdd(Core)

EndDeclareModule()
