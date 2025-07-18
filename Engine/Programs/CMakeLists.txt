add_subdirectory(PointCloud)

file(GLOB PC_SOURCES
    "*.cpp"
    "*.hpp"
)

add_executable(pc ${PC_SOURCES})
target_compile_features(pc PRIVATE cxx_std_20)
target_link_libraries(pc PRIVATE
    Worse::Engine
    Worse::Renderer
    Worse::ECS
    Worse::PointCloud
)