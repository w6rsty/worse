include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(Engine)

    Deprecated()

    PrivateDependencyModuleAdd(Common)
    PrivateDependencyModuleAdd(Core)
    PrivateDependencyModuleAdd(ECS)
    PrivateDependencyModuleAdd(RHI)
    PrivateDependencyModuleAdd(Renderer)

EndDeclareModule()
