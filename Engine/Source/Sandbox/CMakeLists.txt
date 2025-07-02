add_subdirectory(PointCloud)

add_executable(main
    Entry.cpp
    World.hpp
    Editor.cpp
    Functions.cpp
    Keys.cpp
)
target_compile_features(main PRIVATE cxx_std_20)
target_link_libraries(main PRIVATE
    Worse::Engine
    Worse::Renderer
    Worse::ECS
    Worse::PointCloud
)

add_executable(ecs_perf ECS/ecs_perf.cpp)
target_compile_features(ecs_perf PRIVATE cxx_std_20)
target_link_libraries(ecs_perf PRIVATE
    Worse::Core
    Worse::ECS
)