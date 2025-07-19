file(GLOB_RECURSE POINT_CLOUD_HEADERS "*.hpp")
file(GLOB_RECURSE POINT_CLOUD_SOURCE "*.cpp")

# find Open3D
find_package(Open3D REQUIRED)

add_library(PointCloud STATIC ${POINT_CLOUD_HEADERS} ${POINT_CLOUD_SOURCE})
add_library(Worse::PointCloud ALIAS PointCloud)

target_include_directories(PointCloud PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(PointCloud PUBLIC
    Worse::Core
    Worse::ECS
    Worse::Renderer
    LASlib
    Open3D::Open3D
)
