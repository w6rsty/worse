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
        ImGui::StyleColorsDark();

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

        ImGui::Begin("My Window");
        ImGui::Text("Hello, world!");
        ImGui::End();

        ImGui::Render();
    }

} // namespace worse