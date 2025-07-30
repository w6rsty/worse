add_subdirectory(Asset)
add_subdirectory(Core)
add_subdirectory(ECS)
add_subdirectory(Engine)
add_subdirectory(FileSystem)
add_subdirectory(Renderer)
add_subdirectory(RHI)

set_target_properties(
    Asset Core ECS Engine FileSystem Renderer RHI
    PROPERTIES FOLDER "Engine-Runtime"
)