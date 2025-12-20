include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(Common)

    PrivateDependencyModuleAdd(Core)

    PrivateDependencyModuleAdd(SDL3::SDL3)

EndDeclareModule()
