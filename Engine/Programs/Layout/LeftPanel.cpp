#include "imgui.h"
#include "../Application/World.hpp"

void World::leftPanel(ecs::Commands commands)
{
    ImGuiIO& io     = ImGui::GetIO();
    ImVec2 viewport = io.DisplaySize;

    LayoutData* layoutData = commands.getResource<LayoutData>().get();
    float const panelWidth = layoutData->leftPanelWidth;

    // =========================================================================
    // Style
    // =========================================================================

    ImVec4 const cleanBtFg(0.8f, 0.2f, 0.2f, 1.0f); // 红色背景
    ImVec4 const cleanBtHv(1.0f, 0.3f, 0.3f, 1.0f); // 悬停时更亮的红色
    ImVec4 const cleanBtAc(0.6f, 0.1f, 0.1f, 1.0f); // 按下时更深的红色

    ImVec4 const normalTxFg(0.7f, 0.7f, 0.7f, 1.0f);      // 灰色
    ImVec4 const loadedMeshTxFg(1.0f, 0.8f, 0.0f, 1.0f);  // 橙色
    ImVec4 const loadedCloudTxFg(0.0f, 0.6f, 0.0f, 1.0f); // 深绿色

    // =========================================================================
    // Layout
    // =========================================================================

    ImVec2 const panelPos  = ImVec2(5.0f, 5.0f);
    ImVec2 const panelSize = ImVec2(panelWidth - 5.0f, viewport.y - 10.0f);

    ImGui::SetNextWindowPos(panelPos);
    ImGui::SetNextWindowSize(panelSize);
    ImGui::Begin("FileSelector", nullptr, layoutData->leftPanelFlags);
    {
        ImGui::Text("点云文件选择器");
        ImGui::Separator();

        // 清理按钮
        ImGui::PushStyleColor(ImGuiCol_Button, cleanBtFg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, cleanBtHv);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, cleanBtAc);

        static bool showConfirmDialog = false;

        if (ImGui::Button("清理所有网格", ImVec2(-1.0f, 0.0f)))
        {
            if (loadedMeshes.empty())
            {
                WS_LOG_INFO("UI", "No meshes to clear");
            }
            else
            {
                showConfirmDialog = true;
            }
        }

        ImGui::PopStyleColor(3);

        // 确认对话框
        {
            if (showConfirmDialog)
            {
                ImGui::OpenPopup("确认清理");
            }

            if (ImGui::BeginPopupModal("确认清理",
                                       nullptr,
                                       ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("确定要清理所有已加载的网格吗？");
                ImGui::Text("这将释放 %zu 个已加载的网格。", loadedMeshes.size());
                ImGui::Separator();

                if (ImGui::Button("确定", ImVec2(120, 0)))
                {
                    clearAllLoadedMeshes(commands);
                    WS_LOG_INFO("UI", "User confirmed mesh cleanup");
                    showConfirmDialog = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("取消", ImVec2(120, 0)))
                {
                    showConfirmDialog = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

        // 显示已加载网格的统计信息
        if (!loadedMeshes.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, loadedMeshTxFg);
            ImGui::Text("已加载 %zu 个网格", loadedMeshes.size());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();

        // 显示固定的文件夹路径
        ImGui::Text("数据目录:");
        ImGui::PushStyleColor(ImGuiCol_Text, normalTxFg);
        ImGui::TextWrapped("%s", World::POINT_CLOUD_DIRECTORY.c_str());
        ImGui::PopStyleColor();

        ImGui::Spacing();

        // 点云状态显示
        if (hasCloud)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, loadedCloudTxFg);
            ImGui::Text("点云已加载");
            ImGui::PopStyleColor();
            ImGui::Text("点数量: %zu", cloudData.points.size());
        }

        ImGui::Separator();

        // 文件列表
        ImGui::BeginChild("FileList", ImVec2(0, 0), false);
        {
            static int selectedFile = -1;

            for (int i = 0; i < static_cast<int>(availableFiles.size()); i++)
            {
                std::string const& filename = availableFiles[i];

                // 检查当前是否是加载的文件
                bool isCurrentFile = false;
                if (hasCloud && cloudEntity != ecs::Entity::null())
                {
                    // 可以通过检查filename来确定，这里简化处理
                    isCurrentFile = (selectedFile == i);
                }

                // 检查文件是否已加载网格
                bool isMeshLoaded =
                    (loadedMeshes.find(filename) != loadedMeshes.end());

                // 高亮当前加载的文件
                if (isCurrentFile)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, loadedCloudTxFg);
                }
                // 显示已加载网格的文件
                else if (isMeshLoaded)
                {
                    ImGui::PushStyleColor(
                        ImGuiCol_Text,
                        ImVec4(0.0f, 0.4f, 0.8f, 1.0f)); // 深蓝色
                }

                // 构建显示文本
                std::string displayText = filename;
                if (isMeshLoaded)
                {
                    displayText += " [已加载]";
                }

                if (ImGui::Selectable(displayText.c_str(), selectedFile == i))
                {
                    selectedFile = i;

                    // 直接切换到选定的点云（支持运行时加载）
                    bool success = switchToPointCloud(filename, commands);
                    if (success)
                    {
                        WS_LOG_INFO("UI",
                                    "Successfully switched to point cloud: {}",
                                    filename);
                    }
                    else
                    {
                        WS_LOG_ERROR("UI",
                                     "Failed to switch to point cloud: {}",
                                     filename);
                    }
                }

                if (isCurrentFile || isMeshLoaded)
                {
                    ImGui::PopStyleColor();
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}