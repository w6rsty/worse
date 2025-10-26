include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(ECS)

    PrivateDependencyModuleAdd(Core)

EndDeclareModule()
