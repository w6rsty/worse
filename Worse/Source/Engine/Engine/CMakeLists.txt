include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(Engine)

    PrivateDependencyModuleAdd(Core)
    PrivateDependencyModuleAdd(ECS)
    PrivateDependencyModuleAdd(RHI)
    PrivateDependencyModuleAdd(Renderer)

EndDeclareModule()
