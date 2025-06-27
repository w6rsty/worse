add_executable(main main.cpp)
target_compile_features(main PRIVATE cxx_std_20)
target_link_libraries(main PRIVATE
    Worse::Engine
)

add_executable(math math.cpp)
target_compile_features(math PRIVATE cxx_std_20)
target_link_libraries(math PRIVATE
    Worse::Core
)

add_executable(ecs ecs.cpp)
target_compile_features(ecs PRIVATE cxx_std_20)
target_link_libraries(ecs PRIVATE
    Worse::Core
    Worse::ECS
)

add_executable(ecs_perf ecs_perf.cpp)
target_compile_features(ecs_perf PRIVATE cxx_std_20)
target_link_libraries(ecs_perf PRIVATE
    Worse::Core
    Worse::ECS
)