#include "imgui.h"
#include "../Operation/View.hpp"
#include "../Application/World.hpp"

void World::toolbarPanel(ecs::Commands commands,
                         ecs::Resource<GlobalContext> context)
{
    ImGuiIO& io     = ImGui::GetIO();
    ImVec2 viewport = io.DisplaySize;

    LayoutData* layoutData      = commands.getResource<LayoutData>().get();
    float const leftPanelWidth  = layoutData->leftPanelWidth;
    float const rightPanelWidth = layoutData->rightPanelWidth;
    float const toolbarHeight   = layoutData->toolbarHeight;

    float const centerStartX = leftPanelWidth;
    float const centerWidth  = viewport.x - leftPanelWidth - rightPanelWidth;

    Camera* camera             = commands.getResource<Camera>().get();
    CameraData* cameraData     = commands.getResource<CameraData>().get();
    pc::Cloud const& cloudData = cloudStorageManager.get(POINT_CLOUD_DIRECTORY + currentActiveFile)->getMasterCloud();

    // =========================================================================
    // Layout
    // =========================================================================

    ImGui::SetNextWindowPos(ImVec2(centerStartX + 10.0f, 5.0f));
    ImGui::SetNextWindowSize(ImVec2(centerWidth - 20.0f, toolbarHeight));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.7f));
    ImGui::Begin("TopToolbar", nullptr, layoutData->toolbarFlags);
    {
        // 半透明按钮样式
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.3f, 0.3f, 0.3f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

        ImGui::SameLine();
        if (ImGui::Button("适配视图"))
        {
            fitView(camera, cameraData, math::Vector3::ZERO(), cloudData.volume.getExtent().elementMax());
        }
        ImGui::SameLine();

        bool isActive = context->isWireFrameMode;
        if (isActive)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.7f, 0.4f, 1.0f));
        }
        if (ImGui::Button("线框模式"))
        {
            context->isWireFrameMode = !context->isWireFrameMode;
        }
        if (isActive)
        {
            ImGui::PopStyleColor(3);
        }
        ImGui::SameLine();

        // 添加视角模式切换
        static const char* viewModes[] = {"透视", "正交", "顶视", "侧视"};
        static int currentViewMode     = 0;
        static int previousViewMode    = -1; // 用于检测变化

        ImGui::SetNextItemWidth(100);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Combo("##视角",
                         &currentViewMode,
                         viewModes,
                         IM_ARRAYSIZE(viewModes)))
        {
            float const scale = cloudData.volume.getExtent().elementMax();

            // 检测到视角模式变化，切换相机视角
            switch (currentViewMode)
            {
            case 0: // 透视
                setPerspectiveView(camera);
                break;
            case 1: // 正交
                setOrthographicView(camera, scale);
                break;
            case 2: // 顶视
                setTopView(camera, cameraData, scale);
                break;
            case 3: // 侧视
                setSideView(camera, cameraData, scale);
                break;
            }
            previousViewMode = currentViewMode;
        }
        ImGui::PopStyleColor();

        ImGui::PopStyleColor(4);
    }
    ImGui::End();
    ImGui::PopStyleColor();
}
