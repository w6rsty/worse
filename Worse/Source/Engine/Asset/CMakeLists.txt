include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(Asset)

    PrivateDependencyModuleAdd(Core)
    PrivateDependencyModuleAdd(FileSystem)
    PrivateDependencyModuleAdd(stb)

EndDeclareModule()
