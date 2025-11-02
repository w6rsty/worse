include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(Core)

    PrivateDependencyModuleAdd(Common)

    PrivateDependencyModuleAdd(SDL)

EndDeclareModule()
