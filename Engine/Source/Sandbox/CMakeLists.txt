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