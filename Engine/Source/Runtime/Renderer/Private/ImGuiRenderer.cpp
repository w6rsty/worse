#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"

#include "Types.hpp"
#include "Event.hpp"
#include "Window.hpp"
#include "RHIDevice.hpp"
#include "Renderer.hpp"
#include "ImGuiRenderer.hpp"

namespace worse
{

    void defaultPage(int state)
    {
        // clang-format off
        ImVec2 windowSize = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos(
            ImVec2(windowSize.x * 0.5f, windowSize.y * 0.5f),
            ImGuiCond_Always,
            ImVec2(0.5f, 0.5f));
        ImGui::Begin("#",
                        nullptr,
                        ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_AlwaysAutoResize |
                        ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoTitleBar
                );
        ImGui::Text("No page registered for state: %d", state);
        ImGui::End();
        // clang-format on
    }

    namespace
    {
        RHINativeHandle imguiPool = {};
    }

    void ImGuiRenderer::initialize()
    {
        // clang-format off
        imguiPool = RHIDevice::createImGuiPool(512, 512);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        io.Fonts->AddFontFromFileTTF(
            "/Users/w6rsty/dev/Cpp/worse/Engine/Binary/Fonts/NotoSerifSC-Regular.ttf",
            18.0f,
            nullptr,
            io.Fonts->GetGlyphRangesChineseSimplifiedCommon()
        );

        ImGuiStyle& style = ImGui::GetStyle();

        // === Rounding ===
        style.WindowRounding    = 5.0f;
        style.ChildRounding     = 5.0f;
        style.FrameRounding     = 4.0f;
        style.PopupRounding     = 5.0f;
        style.ScrollbarRounding = 12.0f;
        style.GrabRounding      = 4.0f;
        style.TabRounding       = 4.0f;

        // === Borders ===
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize  = 1.0f;
        style.PopupBorderSize  = 1.0f;
        style.FrameBorderSize  = 1.0f;
        style.TabBorderSize    = 1.0f;

        // === Spacing ===
        style.WindowPadding    = ImVec2(10.0f, 10.0f);
        style.FramePadding     = ImVec2(8.0f, 6.0f);
        style.ItemSpacing      = ImVec2(8.0f, 4.0f);
        style.ItemInnerSpacing = ImVec2(6.0f, 6.0f);

        // === Colors ===
        ImVec4* colors = style.Colors;

        // Main
        colors[ImGuiCol_Text]                 = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_WindowBg]             = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
        colors[ImGuiCol_ChildBg]              = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
        colors[ImGuiCol_PopupBg]              = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);

        // Borders
        colors[ImGuiCol_Border]               = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
        colors[ImGuiCol_BorderShadow]         = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);

        // Frame controls (input fields, etc.)
        colors[ImGuiCol_FrameBg]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);

        // Title, Headers, and Menus
        colors[ImGuiCol_TitleBg]              = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
        colors[ImGuiCol_TitleBgActive]        = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
        colors[ImGuiCol_MenuBarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);

        // Scrollbar
        colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);

        // Check Mark & Sliders
        colors[ImGuiCol_CheckMark]            = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_SliderGrab]           = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

        // Buttons
        colors[ImGuiCol_Button]               = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
        colors[ImGuiCol_ButtonActive]         = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);

        // Headers (CollapsingHeader, etc.)
        colors[ImGuiCol_Header]               = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
        colors[ImGuiCol_HeaderHovered]        = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
        colors[ImGuiCol_HeaderActive]         = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);

        // Tabs
        colors[ImGuiCol_Tab]                  = colors[ImGuiCol_Button];
        colors[ImGuiCol_TabHovered]           = colors[ImGuiCol_ButtonHovered];
        colors[ImGuiCol_TabActive]            = colors[ImGuiCol_HeaderActive];
        colors[ImGuiCol_TabUnfocused]         = ImVec4(0.92f, 0.92f, 0.92f, 0.97f);
        colors[ImGuiCol_TabUnfocusedActive]   = colors[ImGuiCol_TabActive];
        
        // Other elements
        colors[ImGuiCol_Separator]            = colors[ImGuiCol_Border];
        colors[ImGuiCol_ResizeGrip]           = ImVec4(0.8f, 0.8f, 0.8f, 0.5f);
        colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.8f, 0.8f, 0.8f, 0.7f);
        colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.8f, 0.8f, 0.8f, 0.9f);

        WS_ASSERT(ImGui_ImplSDL3_InitForVulkan(static_cast<SDL_Window*>(Window::getHandleSDL())));
    
        ImGui_ImplVulkan_LoadFunctions(
            RHIContext::version,
            [](char const* name, void* userData)
            {   
                return vkGetInstanceProcAddr(static_cast<VkInstance>(userData), name);
            },
            RHIContext::instance
        );

        ImGui_ImplVulkan_InitInfo infoInit = {};
        infoInit.Instance            = RHIContext::instance;
        infoInit.PhysicalDevice      = RHIContext::physicalDevice;
        infoInit.Device              = RHIContext::device;
        infoInit.QueueFamily         = RHIDevice::getQueueIndex(RHIQueueType::Graphics);
        infoInit.Queue               = RHIDevice::getQueueHandle(RHIQueueType::Graphics).asValue<VkQueue>();
        infoInit.PipelineCache       = VK_NULL_HANDLE;
        infoInit.DescriptorPool      = imguiPool.asValue<VkDescriptorPool>();
        infoInit.Subpass             = 0;
        infoInit.MinImageCount       = 2;
        infoInit.ImageCount          = 2;
        infoInit.MSAASamples         = VK_SAMPLE_COUNT_1_BIT;
        infoInit.Allocator           = nullptr;
        infoInit.CheckVkResultFn     = [](VkResult err) { WS_ASSERT_VK(err); };
        infoInit.UseDynamicRendering = true;

        VkFormat colorFormats[] = {
            vulkanFormat(Renderer::getRenderTarget(RendererTarget::Output)->getFormat())
        };

        VkPipelineRenderingCreateInfoKHR infoRendering = {};
        infoRendering.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        infoRendering.colorAttachmentCount    = 1;
        infoRendering.pColorAttachmentFormats = colorFormats;
        infoRendering.depthAttachmentFormat   = VK_FORMAT_UNDEFINED;

        infoInit.PipelineRenderingCreateInfo = infoRendering;

        WS_ASSERT(ImGui_ImplVulkan_Init(&infoInit));

        EventBus::subscribe(EventType::SDL, [](Event const& playload) {
            SDL_Event* event = static_cast<SDL_Event*>(std::get<void*>(playload));
            ImGui_ImplSDL3_ProcessEvent(event);
        });
        // clang-format on
    } // namespace worse

    void ImGuiRenderer::shutdown()
    {
        RHIDevice::queueWaitAll();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        RHIDevice::deletionQueueAdd(imguiPool);
    }

    void ImGuiRenderer::tick(ecs::Commands commands,
                             ecs::Resource<GlobalContext> globalContext)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        if (activePage)
        {
            activePage(commands, globalContext);
        }

        for (Page const& page : alwaysRenderPages)
        {
            page(commands, globalContext);
        }

        ImGui::Render();
    }

} // namespace worse