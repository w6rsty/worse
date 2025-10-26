include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(Renderer)

    PublicDependencyModuleAdd(
        Core
        RHI
        ECS
        imgui
        fastgltf)

EndDeclareModule()