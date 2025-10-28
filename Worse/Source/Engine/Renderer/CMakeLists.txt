include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(Renderer)

    PrivateDependencyModuleAdd(Core)
    PrivateDependencyModuleAdd(ECS)
    PrivateDependencyModuleAdd(RHI)

    PrivateDependencyModuleAdd(imgui)
    PrivateDependencyModuleAdd(fastgltf)

EndDeclareModule()