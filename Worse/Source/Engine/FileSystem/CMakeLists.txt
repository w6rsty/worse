include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(FileSystem)

    Deprecated()

    PrivateDependencyModuleAdd(Common)
    PrivateDependencyModuleAdd(Core)

EndDeclareModule()
