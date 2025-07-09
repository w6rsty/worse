add_executable(Example00 Example00.cpp)
target_compile_features(Example00 PRIVATE cxx_std_20)
target_link_libraries(Example00 PRIVATE
    Worse::Engine
    Worse::Renderer
    Worse::ECS
)

add_executable(ecs_perf ECS/ecs_perf.cpp)
target_compile_features(ecs_perf PRIVATE cxx_std_20)
target_link_libraries(ecs_perf PRIVATE
    Worse::Core
    Worse::ECS
)