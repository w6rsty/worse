#include "Cloud.hpp"
#include "imgui.h"
#include "../Application/World.hpp"

void property(ecs::Commands commands, ecs::Entity cloudEntity,
              pc::Cloud& cloudData)
{
    static float pos[3] = {0.0f, 0.0f, 0.0f};
    static float rot[3] = {0.0f, 0.0f, 0.0f};
    static float scale  = 1.0f;

    LocalTransform& xform = commands.getComponent<LocalTransform>(cloudEntity);
    ImGui::Text("变换:");
    if (ImGui::DragFloat3("位置", pos, 0.1f))
    {
        xform.position = math::Vector3{pos[0], pos[1], pos[2]};
    }
    if (ImGui::DragFloat3("旋转", rot, 0.1f, -180.0f, 180.0f))
    {
        xform.rotation =
            math::Quaternion::fromEuler(math::Vector3{math::toRadians(rot[0]),
                                                      math::toRadians(rot[1]),
                                                      math::toRadians(rot[2])});
    }
    if (ImGui::DragFloat("缩放", &scale, 0.1f, 0.2f, 10.0f))
    {
        xform.scale = math::Vector3{scale, scale, scale};
    }

    ImGui::Text("数量: %zu", cloudData.points.size());

    math::Vector3 bbMin  = cloudData.volume.getMin();
    math::Vector3 bbMax  = cloudData.volume.getMax();
    math::Vector3 bbSize = cloudData.volume.getSize();

    ImGui::Text("from: (%.3f, %.3f, %.3f)", bbMin.x, bbMin.y, bbMin.z);
    ImGui::Text("to: (%.3f, %.3f, %.3f)", bbMax.x, bbMax.y, bbMax.z);
}

void World::rightPanel(ecs::Commands commands)
{
    ImGuiIO& io     = ImGui::GetIO();
    ImVec2 viewport = io.DisplaySize;

    LayoutData* layoutData      = commands.getResource<LayoutData>().get();
    float const leftPanelWidth  = layoutData->leftPanelWidth;
    float const rightPanelWidth = layoutData->rightPanelWidth;
    float const centerWidth     = viewport.x - leftPanelWidth - rightPanelWidth;

    ImVec4 processBtFg(0.2f, 0.6f, 0.8f, 1.0f);
    ImVec4 processBtHv(0.3f, 0.7f, 0.9f, 1.0f);
    ImVec4 processBtAc(0.1f, 0.5f, 0.7f, 1.0f);

    // =========================================================================
    // Layout
    // =========================================================================

    ImGui::SetNextWindowPos(ImVec2(leftPanelWidth + centerWidth, 5.0f));
    ImGui::SetNextWindowSize(
        ImVec2(rightPanelWidth - 5.0f, viewport.y - 10.0f));
    ImGui::Begin("PropertyInspector", nullptr, layoutData->rightPanelFlags);
    {
        ImGui::Text("属性查看器");
        ImGui::Separator();

        if (hasCloud &&
            ImGui::CollapsingHeader("属性", ImGuiTreeNodeFlags_DefaultOpen))
        {
            property(commands, cloudEntity, cloudData);
        }

        if (hasCloud &&
            ImGui::CollapsingHeader("处理", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Separator();

            // 显示当前活跃文件
            if (!currentActiveFile.empty())
            {
                ImGui::Text("%s", currentActiveFile.c_str());
                ImGui::Spacing();
            }

            if (isProcessing)
            {
                ImGui::PushStyleColor(ImGuiCol_Text,
                                      ImVec4(0.0f, 0.8f, 1.0f, 1.0f)); // 蓝色
                ImGui::Text("正在处理: %s", currentProcessingFile.c_str());
                ImGui::PopStyleColor();
                ImGui::Spacing();
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, processBtFg);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, processBtHv);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, processBtAc);

                if (ImGui::Button("开始处理", ImVec2(-1.0f, 0.0f)))
                {
                    if (!currentActiveFile.empty())
                    {
                        bool success = processMesh(currentActiveFile, commands);
                        if (success)
                        {
                            WS_LOG_INFO("UI",
                                        "Started processing for: {}",
                                        currentActiveFile);
                        }
                        else
                        {
                            WS_LOG_ERROR("UI",
                                         "Failed to start processing for: {}",
                                         currentActiveFile);
                        }
                    }
                    else
                    {
                        WS_LOG_WARN(
                            "UI",
                            "No active point cloud found for processing");
                    }
                }

                ImGui::PopStyleColor(3);

                ImGui::Spacing();
            }
        }
    }
    ImGui::End();
}
