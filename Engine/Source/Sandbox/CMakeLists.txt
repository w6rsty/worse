add_executable(main main.cpp)
target_compile_features(main PRIVATE cxx_std_20)
target_link_libraries(main PRIVATE
    Engine
    Core
)

add_executable(shader shader.cpp)
target_compile_features(shader PRIVATE cxx_std_20)
target_link_libraries(shader PRIVATE
    RHI
    Core
)