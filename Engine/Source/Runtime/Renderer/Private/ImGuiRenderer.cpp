#include "Event.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"

#include "Window.hpp"
#include "RHIDefinitions.hpp"
#include "RHIDevice.hpp"
#include "Renderer.hpp"
#include "ImGuiRenderer.hpp"

namespace worse
{

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

        ImGui::StyleColorsLight();
        
        ImGuiStyle& style = ImGui::GetStyle();
        // Modern rounded corners
        style.WindowRounding    = 8.0f;
        style.ChildRounding     = 6.0f;
        style.FrameRounding     = 4.0f;
        style.PopupRounding     = 6.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding      = 4.0f;
        style.TabRounding       = 4.0f;
        
        // 1px borders/outlines
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize  = 1.0f;
        style.PopupBorderSize  = 1.0f;
        style.FrameBorderSize  = 1.0f;
        style.TabBorderSize    = 1.0f;
        
        // Modern spacing
        style.WindowPadding    = ImVec2(12, 12);
        style.FramePadding     = ImVec2(8, 4);
        style.ItemSpacing      = ImVec2(8, 6);
        style.ItemInnerSpacing = ImVec2(6, 4);
        
        // Light theme color adjustments
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_Border]         = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
        colors[ImGuiCol_WindowBg]       = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);
        colors[ImGuiCol_FrameBg]        = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
        colors[ImGuiCol_FrameBgActive]  = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);

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

        createBasicLayout(globalContext);

        ImGui::Render();
    }

    void
    ImGuiRenderer::createBasicLayout(ecs::Resource<GlobalContext> globalContext)
    {
        // clang-format off
        ImGuiViewport const* viewport = ImGui::GetMainViewport();
        ImVec2 viewportPadding(5.0f, 5.0f);

        // Define panel dimensions
        float hierarchyWidth  = viewport->WorkSize.x * 0.20f;
        float propertiesWidth = viewport->WorkSize.x * 0.25f;

        // Hierarchy panel (left side)
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewportPadding.x, viewport->WorkPos.y + viewportPadding.y));
        ImGui::SetNextWindowSize(ImVec2(hierarchyWidth - viewportPadding.x, viewport->WorkSize.y - viewportPadding.y * 2));
        createHierarchyPanel();

        // Properties panel (right side)
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - propertiesWidth, viewport->WorkPos.y + viewportPadding.y));
        ImGui::SetNextWindowSize(ImVec2(propertiesWidth - viewportPadding.x, viewport->WorkSize.y - viewportPadding.y * 2));

        createPropertiesPanel();
        // clang-format on
    }

    void ImGuiRenderer::createHierarchyPanel()
    {
        ImGui::Begin("场景层级");

        if (ImGui::TreeNode("Scene Root"))
        {
            if (ImGui::TreeNode("Lighting"))
            {
            }

            if (ImGui::TreeNode("Geometry"))
            {
            }

            if (ImGui::TreeNode("Characters"))
            {
            }

            ImGui::TreePop();
        }

        ImGui::End();
    }

    void ImGuiRenderer::createPropertiesPanel()
    {
        ImGui::Begin("属性");

        ImGui::End();
    }

} // namespace worse