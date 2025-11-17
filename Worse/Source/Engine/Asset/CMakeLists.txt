include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(Asset)

    Deprecated()

    PrivateDependencyModuleAdd(Common)
    PrivateDependencyModuleAdd(Core)
    PrivateDependencyModuleAdd(FileSystem)
    
    PrivateDependencyModuleAdd(stb)

EndDeclareModule()
