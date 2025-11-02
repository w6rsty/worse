include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(FileSystem)

    PrivateDependencyModuleAdd(Common)
    PrivateDependencyModuleAdd(Core)

EndDeclareModule()
