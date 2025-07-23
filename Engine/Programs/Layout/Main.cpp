#include "imgui.h"

#include "../Application/World.hpp"

#include <limits>

void World::mainLayout(ecs::Commands commands,
                       ecs::Resource<GlobalContext> context)
{
    LayoutData* layoutData = commands.getResource<LayoutData>().get();
    if (!layoutData->isVisible)
    {
        return;
    }

    // 获取视口大小
    ImGuiIO& io     = ImGui::GetIO();
    ImVec2 viewport = io.DisplaySize;

    // clang-format off
    /*+--------------------------------------------------------------------------------------------+*/
    /*|*/ leftPanel(commands); /*|*/ toolbarPanel(commands, context); /*|*/ rightPanel(commands);/*|*/
    /*|*/                      /*|*/                                  /*|*/                      /*|*/ 
    /*|*/                      /*|*/                                  /*|*/                      /*|*/ 
    /*|*/                      /*|*/ statusPanel(commands);           /*|*/                      /*|*/    
    /*+--------------------------------------------------------------------------------------------+*/
    // clang-format on

    {
        // 5. 电力线参数弹出窗口
        if (layoutData->showPowerLineParamsPopup)
        {
            ImGui::OpenPopup("电力线参数分析结果");
            layoutData->showPowerLineParamsPopup = false; // 只打开一次
        }

        // 设置弹出窗口的大小和位置（居中显示）
        ImVec2 center = ImVec2(viewport.x * 0.5f, viewport.y * 0.5f);
        ImGui::SetNextWindowPos(center,
                                ImGuiCond_Appearing,
                                ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(800, 700), ImGuiCond_Appearing);

        if (ImGui::BeginPopupModal("电力线参数分析结果",
                                   nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (!World::lastProcessedFile.empty())
            {
                ImGui::Text("处理文件: %s", World::lastProcessedFile.c_str());
            }

            // 分析统计信息
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.1f, 1.0f)); // 亮黄色
            ImGui::Text("识别到 %zu 条电力线", World::powerLineParams.size());
            ImGui::PopStyleColor();

            // 计算总体统计信息
            if (!World::powerLineParams.empty())
            {
                float totalLength = 0.0f;
                float avgSag      = 0.0f;
                float minHeight   = std::numeric_limits<float>::max();
                float maxHeight   = std::numeric_limits<float>::lowest();

                for (const auto& param : World::powerLineParams)
                {
                    totalLength += param.length;
                    avgSag += param.maxSag;
                    float avgHeight = (param.startZ + param.endZ) / 2.0f;
                    minHeight       = std::min(minHeight, avgHeight);
                    maxHeight       = std::max(maxHeight, avgHeight);
                }
                avgSag /= World::powerLineParams.size();

                ImGui::Text("总线路长度: %.1f m", totalLength);
                ImGui::Text("平均弧垂: %.3f m", avgSag);
                ImGui::Text("高度范围: %.1f - %.1f m", minHeight, maxHeight);
            }
            ImGui::Separator();
            ImGui::Spacing();

            // 主要参数表格
            if (!World::powerLineParams.empty())
            {
                ImGui::PushStyleColor(ImGuiCol_Text,
                                      ImVec4(0.8f, 0.8f, 0.4f, 1.0f)); // 黄色
                ImGui::Text("🔧 电力线形态特征:");
                ImGui::PopStyleColor();

                if (ImGui::BeginTable(
                        "MainTable",
                        4, // 简化为4列: UUID, 长度, 弧垂, 导线规格
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY,
                        ImVec2(0, 220)))
                {
                    // 表头设置
                    ImGui::TableSetupColumn("UUID",
                                            ImGuiTableColumnFlags_WidthFixed,
                                            120);
                    ImGui::TableSetupColumn("长度(m)",
                                            ImGuiTableColumnFlags_WidthFixed,
                                            80);
                    ImGui::TableSetupColumn("最大弧垂(m)",
                                            ImGuiTableColumnFlags_WidthFixed,
                                            90);
                    ImGui::TableSetupColumn("导线规格(mm)",
                                            ImGuiTableColumnFlags_WidthFixed,
                                            100);

                    ImGui::TableSetupScrollFreeze(0, 1);

                    // 表头样式 - 增强对比度
                    ImGui::PushStyleColor(
                        ImGuiCol_TableHeaderBg,
                        ImVec4(0.3f, 0.5f, 0.7f, 1.0f)); // 蓝色背景
                    ImGui::PushStyleColor(
                        ImGuiCol_Text,
                        ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // 白色文字
                    ImGui::TableHeadersRow();
                    ImGui::PopStyleColor(2);

                    // 表格内容
                    for (const auto& param : World::powerLineParams)
                    {
                        ImGui::TableNextRow();

                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s", param.name.c_str());

                        ImGui::TableSetColumnIndex(1);
                        // 根据长度设置颜色
                        if (param.length > 100.0f)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.2f, 1.0f)); // 橙色
                        }
                        else if (param.length > 50.0f)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // 绿色
                        }
                        else
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f)); // 灰色
                        }
                        ImGui::Text("%.1f", param.length);
                        ImGui::PopStyleColor();

                        ImGui::TableSetColumnIndex(2);
                        // 根据弧垂设置颜色 - 弧垂大表示线路松弛
                        if (param.maxSag > 5.0f)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f)); // 红色（需要关注）
                        }
                        else if (param.maxSag > 2.0f)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f)); // 黄色（正常）
                        }
                        else
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // 绿色（良好）
                        }
                        ImGui::Text("%.3f", param.maxSag);
                        ImGui::PopStyleColor();

                        ImGui::TableSetColumnIndex(3);
                        ImGui::Text("%.1f", param.width);
                    }
                    ImGui::EndTable();
                }

                // 详细技术参数展开区域
                if (ImGui::CollapsingHeader("悬链线参数", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::TextWrapped("悬链线方程: y = a × cosh((x - h) / a) + k");
                    ImGui::Text("其中 a=张力系数, h=水平位移, k=垂直位移");
                    ImGui::Separator();

                    if (ImGui::BeginTable("DetailedTable",
                                          5,
                                          ImGuiTableFlags_Borders |
                                              ImGuiTableFlags_RowBg |
                                              ImGuiTableFlags_ScrollX))
                    {
                        ImGui::TableSetupColumn(
                            "UUID",
                            ImGuiTableColumnFlags_WidthFixed,
                            120);
                        ImGui::TableSetupColumn(
                            "张力系数(a)",
                            ImGuiTableColumnFlags_WidthFixed,
                            100);
                        ImGui::TableSetupColumn(
                            "水平位移(h)",
                            ImGuiTableColumnFlags_WidthFixed,
                            100);
                        ImGui::TableSetupColumn(
                            "垂直位移(k)",
                            ImGuiTableColumnFlags_WidthFixed,
                            100);
                        ImGui::TableSetupColumn(
                            "弧垂评级",
                            ImGuiTableColumnFlags_WidthFixed,
                            80);

                        // 表头样式 - 增强对比度
                        ImGui::PushStyleColor(
                            ImGuiCol_TableHeaderBg,
                            ImVec4(0.5f, 0.3f, 0.7f, 1.0f)); // 紫色背景
                        ImGui::PushStyleColor(
                            ImGuiCol_Text,
                            ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // 白色文字
                        ImGui::TableHeadersRow();
                        ImGui::PopStyleColor(2);

                        for (const auto& param : World::powerLineParams)
                        {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s", param.name.c_str());

                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%.2f", param.catenaryA);

                            ImGui::TableSetColumnIndex(2);
                            ImGui::Text("%.1f", param.catenaryH);

                            ImGui::TableSetColumnIndex(3);
                            ImGui::Text("%.1f", param.catenaryK);

                            ImGui::TableSetColumnIndex(4);
                            // 弧垂评级
                            std::string rating;
                            if (param.maxSag > 5.0f)
                            {
                                rating = "需关注";
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                            }
                            else if (param.maxSag > 2.0f)
                            {
                                rating = "正常";
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
                            }
                            else
                            {
                                rating = "良好";
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.8f, 1.0f));
                            }
                            ImGui::Text("%s", rating.c_str());
                            ImGui::PopStyleColor();
                        }
                        ImGui::EndTable();
                    }
                }

                // 坐标信息展开区域
                if (ImGui::CollapsingHeader("坐标信息"))
                {
                    if (ImGui::BeginTable("CoordinateTable",
                                          7,
                                          ImGuiTableFlags_Borders |
                                              ImGuiTableFlags_RowBg |
                                              ImGuiTableFlags_ScrollX))
                    {
                        ImGui::TableSetupColumn(
                            "UUID",
                            ImGuiTableColumnFlags_WidthFixed,
                            120);
                        ImGui::TableSetupColumn(
                            "起点X",
                            ImGuiTableColumnFlags_WidthFixed,
                            80);
                        ImGui::TableSetupColumn(
                            "起点Y",
                            ImGuiTableColumnFlags_WidthFixed,
                            80);
                        ImGui::TableSetupColumn(
                            "起点Z",
                            ImGuiTableColumnFlags_WidthFixed,
                            80);
                        ImGui::TableSetupColumn(
                            "终点X",
                            ImGuiTableColumnFlags_WidthFixed,
                            80);
                        ImGui::TableSetupColumn(
                            "终点Y",
                            ImGuiTableColumnFlags_WidthFixed,
                            80);
                        ImGui::TableSetupColumn(
                            "终点Z",
                            ImGuiTableColumnFlags_WidthFixed,
                            80);

                        // 表头样式 - 增强对比度
                        ImGui::PushStyleColor(
                            ImGuiCol_TableHeaderBg,
                            ImVec4(0.2f, 0.6f, 0.4f, 1.0f)); // 绿色背景
                        ImGui::PushStyleColor(
                            ImGuiCol_Text,
                            ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // 白色文字
                        ImGui::TableHeadersRow();
                        ImGui::PopStyleColor(2);

                        for (const auto& param : World::powerLineParams)
                        {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s", param.name.c_str());

                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%.3f", param.startX);

                            ImGui::TableSetColumnIndex(2);
                            ImGui::Text("%.3f", param.startY);

                            ImGui::TableSetColumnIndex(3);
                            ImGui::Text("%.3f", param.startZ);

                            ImGui::TableSetColumnIndex(4);
                            ImGui::Text("%.3f", param.endX);

                            ImGui::TableSetColumnIndex(5);
                            ImGui::Text("%.3f", param.endY);

                            ImGui::TableSetColumnIndex(6);
                            ImGui::Text("%.3f", param.endZ);
                        }
                        ImGui::EndTable();
                    }
                }
            }
            else
            {
                ImGui::PushStyleColor(
                    ImGuiCol_Text,
                    ImVec4(0.8f, 0.8f, 0.0f, 1.0f)); // 黄色警告
                ImGui::Text("⚠ 未检测到电力线或数据生成失败");
                ImGui::PopStyleColor();
            }

            ImGui::Separator();
            ImGui::Spacing();

            // 操作按钮
            float buttonWidth = 120.0f;
            float totalWidth =
                buttonWidth * 4 + ImGui::GetStyle().ItemSpacing.x * 3;
            float startX =
                (ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f;

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + startX);

            // SQLite导出按钮
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.8f, 0.4f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.9f, 0.5f, 0.3f, 1.0f));
            if (ImGui::Button("生成SQLite", ImVec2(buttonWidth, 0)))
            {
                // 生成SQLite插入语句并输出到日志
                WS_LOG_INFO("SQLite", "=== 电力线数据库插入语句 ===");
                WS_LOG_INFO("SQLite", "-- 创建表结构");
                WS_LOG_INFO("SQLite",
                            "CREATE TABLE IF NOT EXISTS power_lines (");
                WS_LOG_INFO("SQLite", "    id INTEGER PRIMARY KEY,");
                WS_LOG_INFO("SQLite", "    name TEXT NOT NULL,");
                WS_LOG_INFO("SQLite", "    startX REAL,");
                WS_LOG_INFO("SQLite", "    startY REAL,");
                WS_LOG_INFO("SQLite", "    startZ REAL,");
                WS_LOG_INFO("SQLite", "    endX REAL,");
                WS_LOG_INFO("SQLite", "    endY REAL,");
                WS_LOG_INFO("SQLite", "    endZ REAL,");
                WS_LOG_INFO("SQLite", "    length REAL,");
                WS_LOG_INFO("SQLite", "    width REAL,");
                WS_LOG_INFO("SQLite", "    maxSag REAL,");
                WS_LOG_INFO("SQLite", "    catenaryA REAL,");
                WS_LOG_INFO("SQLite", "    catenaryH REAL,");
                WS_LOG_INFO("SQLite", "    catenaryK REAL");
                WS_LOG_INFO("SQLite", ");");
                WS_LOG_INFO("SQLite", "");
                WS_LOG_INFO("SQLite", "-- 插入数据");

                for (const auto& param : World::powerLineParams)
                {
                    WS_LOG_INFO(
                        "SQLite",
                        "INSERT INTO power_lines (id, name, startX, startY, "
                        "startZ, endX, endY, endZ, "
                        "length, width, maxSag, catenaryA, catenaryH, "
                        "catenaryK) VALUES "
                        "({}, '{}', {:.3f}, {:.3f}, {:.3f}, {:.3f}, {:.3f}, "
                        "{:.3f}, "
                        "{:.2f}, {:.1f}, {:.3f}, {:.6f}, {:.3f}, {:.3f});",
                        param.id,
                        param.name,
                        param.startX,
                        param.startY,
                        param.startZ,
                        param.endX,
                        param.endY,
                        param.endZ,
                        param.length,
                        param.width,
                        param.maxSag,
                        param.catenaryA,
                        param.catenaryH,
                        param.catenaryK);
                }
                WS_LOG_INFO("SQLite", "=== SQLite语句生成完成 ===");
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine();

            // 导出CSV按钮
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.2f, 0.6f, 0.8f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.3f, 0.7f, 0.9f, 1.0f));
            if (ImGui::Button("导出CSV", ImVec2(buttonWidth, 0)))
            {
                WS_LOG_INFO("CSV", "=== 电力线参数CSV格式 ===");
                WS_LOG_INFO(
                    "CSV",
                    "ID,Name,StartX,StartY,StartZ,EndX,EndY,EndZ,Length,Width,"
                    "MaxSag,CatenaryA,CatenaryH,CatenaryK");
                for (const auto& param : World::powerLineParams)
                {
                    WS_LOG_INFO("CSV",
                                "{},{},{:.3f},{:.3f},{:.3f},{:.3f},{:.3f},{:."
                                "3f},{:.2f},{:."
                                "1f},{:.3f},{:.6f},{:.3f},{:.3f}",
                                param.id,
                                param.name,
                                param.startX,
                                param.startY,
                                param.startZ,
                                param.endX,
                                param.endY,
                                param.endZ,
                                param.length,
                                param.width,
                                param.maxSag,
                                param.catenaryA,
                                param.catenaryH,
                                param.catenaryK);
                }
                WS_LOG_INFO("CSV", "=== CSV导出完成 ===");
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine();

            // 关闭按钮
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
            if (ImGui::Button("关闭", ImVec2(buttonWidth, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleColor(2);

            ImGui::EndPopup();
        }
    }
}