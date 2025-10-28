add_library(imgui STATIC
    # Core ImGui files
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    
    # Backends for this project (SDL3 + Vulkan)
    backends/imgui_impl_sdl3.cpp
    backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC
    Vulkan-Headers
    SDL
)

target_compile_definitions(imgui PUBLIC
    IMGUI_IMPL_VULKAN_NO_PROTOTYPES  # Use volk for Vulkan function loading
)

if(APPLE)
    target_compile_definitions(imgui PRIVATE IMGUI_IMPL_METAL_CPP)
endif()

source_group("Core" FILES
    imgui.cpp
    imgui.h
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_internal.h
    imgui_tables.cpp
    imgui_widgets.cpp
    imconfig.h
)

source_group("Backends" FILES
    backends/imgui_impl_sdl3.cpp
    backends/imgui_impl_sdl3.h
    backends/imgui_impl_vulkan.cpp
    backends/imgui_impl_vulkan.h
)

source_group("STB Headers" FILES
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
)

# Set library output name
set_target_properties(imgui PROPERTIES
    OUTPUT_NAME "imgui"
    DEBUG_POSTFIX "_d"
)

target_include_directories(imgui INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/backends>
    $<INSTALL_INTERFACE:include/imgui>
    $<INSTALL_INTERFACE:include/imgui/backends>
)

add_library(ImGui::ImGui ALIAS imgui)
