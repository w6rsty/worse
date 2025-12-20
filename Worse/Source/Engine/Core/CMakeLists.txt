include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(Core)

    PrivateDependencyModuleAdd(spdlog::spdlog)

EndDeclareModule()
