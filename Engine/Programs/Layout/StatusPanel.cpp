#include "imgui.h"
#include "../Application/World.hpp"

void World::statusPanel(ecs::Commands commands)
{
    ImGuiIO& io     = ImGui::GetIO();
    ImVec2 viewport = io.DisplaySize;

    LayoutData* layoutData      = commands.getResource<LayoutData>().get();
    float const leftPanelWidth  = layoutData->leftPanelWidth;
    float const rightPanelWidth = layoutData->rightPanelWidth;
    float const statusHeight    = layoutData->statusHeight;

    float const centerStartX = leftPanelWidth;
    float const centerWidth  = viewport.x - leftPanelWidth - rightPanelWidth;

    ecs::Resource<Camera> camera         = commands.getResource<Camera>();
    ecs::Resource<CameraData> cameraData = commands.getResource<CameraData>();

    // =========================================================================
    // Layout
    // =========================================================================

    ImGui::SetNextWindowPos(
        ImVec2(centerStartX + 10.0f, viewport.y - statusHeight - 5.0f));
    ImGui::SetNextWindowSize(ImVec2(centerWidth - 20.0f, statusHeight));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.7f));
    ImGui::Begin("BottomStatus", nullptr, layoutData->statusPanelFlags);
    {
        // 半透明文本和控件样式
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));

        // 使用列布局来更好地组织状态信息
        ImGui::Columns(3, "StatusColumns", false);

        ImGui::SetNextItemWidth(120);
        ImGui::SliderFloat("相机速度", &cameraData->moveSpeed, 0.1f, 50.0f);

        ImGui::NextColumn();
        {
            math::Vector3 camPos = camera->getPosition();
            ImGui::Text("相机位置:");
            ImGui::Text("(%.3f, %.3f, %.3f)", camPos.x, camPos.y, camPos.z);
        }

        ImGui::NextColumn();
        {
            math::Vector3 camForward = camera->getForward();

            ImGui::Text("相机朝向:");
            ImGui::Text("(%.3f, %.3f, %.3f)",
                        camForward.x,
                        camForward.y,
                        camForward.z);
        }

        ImGui::Columns(1); // 恢复单列

        ImGui::PopStyleColor(2);
    }
    ImGui::End();
    ImGui::PopStyleColor();
}
