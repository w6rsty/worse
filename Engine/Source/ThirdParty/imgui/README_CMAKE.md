# ImGui Library Usage

This CMake configuration provides a complete ImGui library setup for use with SDL3 and Vulkan.

## Features

- **Core ImGui**: All essential ImGui functionality including widgets, tables, and drawing
- **SDL3 Backend**: Integration with SDL3 for window management and input handling
- **Vulkan Backend**: Vulkan rendering support using volk for function loading
- **Platform Support**: Cross-platform with macOS-specific optimizations
- **Debug Support**: Optional demo window and debug logging

## CMake Options

```cmake
option(IMGUI_ENABLE_DEMO "Enable ImGui demo window" OFF)
option(IMGUI_ENABLE_DEBUG_LOG "Enable ImGui debug logging" OFF)
```

## Usage in Your Project

1. **Link the library**:
```cmake
target_link_libraries(your_target PRIVATE ImGui::ImGui)
```

2. **Include headers**:
```cpp
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
```

3. **Basic setup** (in your main application):
```cpp
// Initialize ImGui
IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImGuiIO& io = ImGui::GetIO(); (void)io;

// Setup Dear ImGui style
ImGui::StyleColorsDark();

// Setup Platform/Renderer backends
ImGui_ImplSDL3_InitForVulkan(window);
ImGui_ImplVulkan_InitInfo init_info = {};
// Fill in your Vulkan initialization info
ImGui_ImplVulkan_Init(&init_info, render_pass);
```

4. **In your render loop**:
```cpp
// Start the Dear ImGui frame
ImGui_ImplVulkan_NewFrame();
ImGui_ImplSDL3_NewFrame();
ImGui::NewFrame();

// Your ImGui code here
ImGui::Begin("Hello, world!");
ImGui::Text("This is some useful text.");
ImGui::End();

// Rendering
ImGui::Render();
ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
```

## Dependencies

- **SDL3**: Window management and input handling
- **volk**: Vulkan function loader
- **Vulkan**: Graphics API (system dependency)

## Build Configuration

The library automatically enables the demo window in Debug builds. You can override this behavior with the CMake options.

For production builds, make sure to set `IMGUI_ENABLE_DEMO=OFF` to reduce binary size.

## Platform Notes

- **macOS**: Includes Metal C++ support definitions for compatibility
- **All platforms**: Uses volk for Vulkan function loading instead of direct linking

## File Organization

- **Core files**: Main ImGui implementation
- **Backends**: SDL3 and Vulkan backend implementations  
- **Headers**: Public API and internal headers
- **STB headers**: Embedded STB libraries for font/image processing
