include(${CMAKE_SOURCE_DIR}/CMake/BuildTool/BuildTool.cmake)

DeclareModule(Common)

    PublicPch(${CMAKE_CURRENT_SOURCE_DIR}/public/common_pch.hpp)

EndDeclareModule()
