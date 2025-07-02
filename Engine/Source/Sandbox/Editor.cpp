#include "Math/Base.hpp"
#include "imgui.h"
#include "World.hpp"

void World::editorLayout(ecs::Commands commands,
                         ecs::Resource<GlobalContext> context)
{
    // 获取视口大小
    ImGuiIO& io         = ImGui::GetIO();
    ImVec2 viewportSize = io.DisplaySize;

    // 定义面板尺寸（确保精确对齐）
    const float leftPanelWidth  = 250.0f;
    const float rightPanelWidth = 300.0f;
    const float toolbarHeight   = 50.0f;
    const float statusHeight    = 60.0f;

    // 计算中间区域尺寸和位置
    const float centerStartX = leftPanelWidth;
    const float centerWidth = viewportSize.x - leftPanelWidth - rightPanelWidth;
    const float centerHeight = viewportSize.y;

    // 1. 左侧文件选择器面板
    ImGui::SetNextWindowPos(ImVec2(5.0f, 5.0f));
    ImGui::SetNextWindowSize(
        ImVec2(leftPanelWidth - 5.0f, viewportSize.y - 10.0f));
    ImGui::Begin("FileSelector",
                 nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    {
        ImGui::Text("点云文件选择器");
        ImGui::Separator();

        // 清理按钮
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // 红色背景
        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered,
            ImVec4(1.0f, 0.3f, 0.3f, 1.0f)); // 悬停时更亮的红色
        ImGui::PushStyleColor(
            ImGuiCol_ButtonActive,
            ImVec4(0.6f, 0.1f, 0.1f, 1.0f)); // 按下时更深的红色

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

        // 显示已加载网格的统计信息
        if (!loadedMeshes.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4(1.0f, 0.8f, 0.0f, 1.0f)); // 橙色
            ImGui::Text("已加载 %zu 个网格", loadedMeshes.size());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();

        // 显示固定的文件夹路径
        ImGui::Text("数据目录:");
        ImGui::PushStyleColor(ImGuiCol_Text,
                              ImVec4(0.7f, 0.7f, 0.7f, 1.0f)); // 灰色
        ImGui::TextWrapped("%s", World::POINT_CLOUD_DIRECTORY.c_str());
        ImGui::PopStyleColor();

        ImGui::Spacing();

        // 点云状态显示
        if (hasPointCloud)
        {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4(0.0f, 0.6f, 0.0f, 1.0f)); // 深绿色
            ImGui::Text("点云已加载");
            ImGui::PopStyleColor();
            ImGui::Text("点数量: %zu", pointCloudData.points.size());
        }

        // 显示加载进度条（如果正在加载）
        if (isLoadingMesh)
        {
            ImGui::Text("正在加载: %s", currentLoadingFile.c_str());
            ImGui::ProgressBar(loadingProgress, ImVec2(-1.0f, 0.0f));
            ImGui::Spacing();
        }

        ImGui::Separator();

        // 文件列表
        ImGui::Text("可用文件 (%zu 个):", availableFiles.size());
        ImGui::BeginChild("FileList", ImVec2(0, 0), false);
        {
            static int selectedFile = -1;

            for (int i = 0; i < static_cast<int>(availableFiles.size()); i++)
            {
                const std::string& filename = availableFiles[i];

                // 检查当前是否是加载的文件
                bool isCurrentFile = false;
                if (hasPointCloud && pointCloudEntity != ecs::Entity::null())
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
                    ImGui::PushStyleColor(
                        ImGuiCol_Text,
                        ImVec4(0.0f, 0.6f, 0.0f, 1.0f)); // 深绿色
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

            // 如果没有可用文件，显示提示
            if (availableFiles.empty())
            {
                ImGui::PushStyleColor(ImGuiCol_Text,
                                      ImVec4(0.7f, 0.7f, 0.7f, 1.0f)); // 灰色
                ImGui::Text("未找到点云文件...");
                ImGui::PopStyleColor();
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();

    // 2. 右侧属性查看器面板
    ImGui::SetNextWindowPos(ImVec2(leftPanelWidth + centerWidth, 5.0f));
    ImGui::SetNextWindowSize(
        ImVec2(rightPanelWidth - 5.0f, viewportSize.y - 10.0f));
    ImGui::Begin("PropertyInspector",
                 nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    {
        ImGui::Text("属性查看器");
        ImGui::Separator();

        // 点云属性
        if (ImGui::CollapsingHeader("点云属性", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (hasPointCloud)
            {
                ImGui::Text("点云统计:");
                ImGui::Text("点数量: %zu", pointCloudData.points.size());

                // 包围盒信息
                math::Vector3 bbMin  = pointCloudData.boundingBox.getMin();
                math::Vector3 bbMax  = pointCloudData.boundingBox.getMax();
                math::Vector3 bbSize = pointCloudData.boundingBox.getSize();

                ImGui::Text("包围盒:");
                ImGui::Text("  最小: (%.3f, %.3f, %.3f)",
                            bbMin.x,
                            bbMin.y,
                            bbMin.z);
                ImGui::Text("  最大: (%.3f, %.3f, %.3f)",
                            bbMax.x,
                            bbMax.y,
                            bbMax.z);
                ImGui::Text("  尺寸: (%.3f, %.3f, %.3f)",
                            bbSize.x,
                            bbSize.y,
                            bbSize.z);
                ImGui::Text("  中心: (%.3f, %.3f, %.3f)",
                            pointCloudCenter.x,
                            pointCloudCenter.y,
                            pointCloudCenter.z);

                ImGui::Spacing();
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text,
                                      ImVec4(0.7f, 0.7f, 0.7f, 1.0f)); // 灰色
                ImGui::Text("未加载点云数据");
                ImGui::PopStyleColor();
                ImGui::Spacing();
            }
        }

        // PCL处理模块（如果有点云加载）
        if (hasPointCloud &&
            ImGui::CollapsingHeader("PCL处理", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("点云库(PCL)处理工具");
            ImGui::Separator();

            // 显示当前活跃文件
            if (!currentActiveFile.empty())
            {
                ImGui::Text("当前文件: %s", currentActiveFile.c_str());
                ImGui::Spacing();
            }

            // 显示PCL处理进度条（如果正在处理）
            if (isProcessingWithPCL)
            {
                ImGui::PushStyleColor(ImGuiCol_Text,
                                      ImVec4(0.0f, 0.8f, 1.0f, 1.0f)); // 蓝色
                ImGui::Text("正在处理: %s", currentProcessingFile.c_str());
                ImGui::PopStyleColor();
                ImGui::ProgressBar(pclProcessingProgress, ImVec2(-1.0f, 0.0f));
                ImGui::Spacing();
            }
            else
            {
                // PCL处理按钮
                ImGui::PushStyleColor(
                    ImGuiCol_Button,
                    ImVec4(0.2f, 0.6f, 0.8f, 1.0f)); // 蓝色背景
                ImGui::PushStyleColor(
                    ImGuiCol_ButtonHovered,
                    ImVec4(0.3f, 0.7f, 0.9f, 1.0f)); // 悬停时更亮
                ImGui::PushStyleColor(
                    ImGuiCol_ButtonActive,
                    ImVec4(0.1f, 0.5f, 0.7f, 1.0f)); // 按下时更深

                if (ImGui::Button("开始PCL处理", ImVec2(-1.0f, 0.0f)))
                {
                    if (!currentActiveFile.empty())
                    {
                        bool success =
                            processMeshWithPCL(currentActiveFile, commands);
                        if (success)
                        {
                            WS_LOG_INFO("UI",
                                        "Started PCL processing for: {}",
                                        currentActiveFile);
                        }
                        else
                        {
                            WS_LOG_ERROR(
                                "UI",
                                "Failed to start PCL processing for: {}",
                                currentActiveFile);
                        }
                    }
                    else
                    {
                        WS_LOG_WARN(
                            "UI",
                            "No active point cloud found for PCL processing");
                    }
                }

                ImGui::PopStyleColor(3);

                ImGui::Spacing();

                // PCL处理选项
                ImGui::Text("处理选项:");
                static bool enableVoxelFilter      = true;
                static bool enableNormalEstimation = false;
                static bool enableSegmentation     = false;

                ImGui::Checkbox("体素滤波", &enableVoxelFilter);
                ImGui::Checkbox("法线估计", &enableNormalEstimation);
                ImGui::Checkbox("平面分割", &enableSegmentation);

                if (enableVoxelFilter)
                {
                    static float voxelSize = 0.01f;
                    ImGui::SliderFloat("体素大小",
                                       &voxelSize,
                                       0.001f,
                                       0.1f,
                                       "%.3f");
                }

                ImGui::Spacing();
            }
        }

        // 对象变换属性（如果有点云Entity）
        if (hasPointCloud &&
            ImGui::CollapsingHeader("变换属性", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static float position[3] = {0.0f, 0.0f, 0.0f};
            static float rotation[3] = {0.0f, 0.0f, 0.0f};
            static float scale[3]    = {1.0f, 1.0f, 1.0f};

            ImGui::Text("Entity ID: %llu", pointCloudEntity.toEntity());
            ImGui::Spacing();

            LocalTransform& transform =
                commands.getComponent<LocalTransform>(pointCloudEntity);
            ImGui::Text("变换:");
            if (ImGui::DragFloat3("位置", position, 0.1f))
            {
                // 更新位置
                transform.position =
                    math::Vector3{position[0], position[1], position[2]};
            }
            if (ImGui::DragFloat3("旋转", rotation, 0.1f, -180.0f, 180.0f))
            {
                // 更新旋转
                transform.rotation = math::Quaternion::fromEuler(
                    math::Vector3{math::toRadians(rotation[0]),
                                  math::toRadians(rotation[1]),
                                  math::toRadians(rotation[2])});
            }
            if (ImGui::DragFloat3("缩放", scale, 0.1f, 0.1f, 10.0f))
            {
                // 更新缩放
                transform.scale = math::Vector3{scale[0], scale[1], scale[2]};
            }
            ImGui::Spacing();
        }
    }
    ImGui::End();

    // 3. 中间区域的顶部工具栏覆盖层
    // 精确对齐中间渲染区域的顶部边界
    ImGui::SetNextWindowPos(ImVec2(centerStartX + 10.0f, 5.0f));
    ImGui::SetNextWindowSize(ImVec2(centerWidth - 20.0f, toolbarHeight));
    ImGui::PushStyleColor(ImGuiCol_WindowBg,
                          ImVec4(0.1f, 0.1f, 0.1f, 0.7f)); // 轻微半透明背景
    ImGui::Begin("TopToolbar",
                 nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    {
        // 半透明按钮样式
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.3f, 0.3f, 0.3f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

        if (ImGui::Button("重置视角"))
        {
            // 重置相机视角
            resetCameraView(currentCamera);
        }
        ImGui::SameLine();
        if (ImGui::Button("适配视图"))
        {
            // 适配点云到视图
            fitCameraToPointCloud(currentCamera);
        }
        ImGui::SameLine();
        if (ImGui::Button("线框模式"))
        {
            context->isWireFrameMode = !context->isWireFrameMode;
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
            // 检测到视角模式变化，切换相机视角
            switch (currentViewMode)
            {
            case 0: // 透视
                setCameraToPerspectiveView(currentCamera);
                break;
            case 1: // 正交
                setCameraToOrthographicView(currentCamera);
                break;
            case 2: // 顶视
                setCameraToTopView(currentCamera);
                break;
            case 3: // 侧视
                setCameraToSideView(currentCamera);
                break;
            }
            previousViewMode = currentViewMode;
        }
        ImGui::PopStyleColor();

        ImGui::PopStyleColor(4);
    }
    ImGui::End();
    ImGui::PopStyleColor();

    // 4. 中间区域的底部状态栏覆盖层
    // 精确对齐中间渲染区域的底部边界
    ImGui::SetNextWindowPos(
        ImVec2(centerStartX + 10.0f, viewportSize.y - statusHeight - 5.0f));
    ImGui::SetNextWindowSize(ImVec2(centerWidth - 20.0f, statusHeight));
    ImGui::PushStyleColor(ImGuiCol_WindowBg,
                          ImVec4(0.1f, 0.1f, 0.1f, 0.7f)); // 轻微半透明背景
    ImGui::Begin("BottomStatus",
                 nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    {
        // 半透明文本和控件样式
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));

        // 使用列布局来更好地组织状态信息
        ImGui::Columns(3, "StatusColumns", false);

        ImGui::SetNextItemWidth(120);
        ImGui::SliderFloat("相机速度", &cameraMoveSpeed, 0.1f, 10.0f);

        ImGui::NextColumn();
        {
            math::Vector3 camPos = currentCamera->getPosition();
            ImGui::Text("相机位置:");
            ImGui::Text("  (%.3f, %.3f, %.3f)", camPos.x, camPos.y, camPos.z);
        }

        ImGui::NextColumn();
        {
            math::Vector3 camForward = currentCamera->getForward();

            ImGui::Text("相机朝向:");
            ImGui::Text("  (%.3f, %.3f, %.3f)",
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