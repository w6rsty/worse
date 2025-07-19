#include "imgui.h"

#include "../Application/World.hpp"

void World::mainLayout(ecs::Commands commands,
                       ecs::Resource<GlobalContext> context)
{
    // 获取视口大小
    ImGuiIO& io     = ImGui::GetIO();
    ImVec2 viewport = io.DisplaySize;

    LayoutData* layoutData = commands.getResource<LayoutData>().get();

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

            // 标题和文件信息
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // 绿色
            ImGui::Text("电力基础设施分析完成");
            ImGui::PopStyleColor();

            if (!World::lastProcessedFile.empty())
            {
                ImGui::Text("处理文件: %s", World::lastProcessedFile.c_str());
            }
            ImGui::Text("识别到 %zu 条电力线", World::powerLineParams.size());
            ImGui::Separator();
            ImGui::Spacing();

            // 主要参数表格
            if (!World::powerLineParams.empty())
            {
                ImGui::PushStyleColor(ImGuiCol_Text,
                                      ImVec4(0.8f, 0.8f, 0.4f, 1.0f)); // 黄色
                ImGui::Text("电力线主要参数:");
                ImGui::PopStyleColor();

                if (ImGui::BeginTable(
                        "MainTable",
                        5,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY,
                        ImVec2(0, 200)))
                {
                    // 表头设置
                    ImGui::TableSetupColumn("ID",
                                            ImGuiTableColumnFlags_WidthFixed,
                                            40);
                    ImGui::TableSetupColumn("名称(UUID)",
                                            ImGuiTableColumnFlags_WidthFixed,
                                            200);
                    ImGui::TableSetupColumn("长度(m)",
                                            ImGuiTableColumnFlags_WidthFixed,
                                            80);
                    ImGui::TableSetupColumn("宽度(mm)",
                                            ImGuiTableColumnFlags_WidthFixed,
                                            80);
                    ImGui::TableSetupColumn("最大弧垂(m)",
                                            ImGuiTableColumnFlags_WidthFixed,
                                            90);

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
                        ImGui::Text("%d", param.id);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%s", param.name.c_str());

                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%.2f", param.length);

                        ImGui::TableSetColumnIndex(3);
                        ImGui::Text("%.1f", param.width);

                        ImGui::TableSetColumnIndex(4);
                        ImGui::Text("%.3f", param.max_sag);
                    }
                    ImGui::EndTable();
                }

                // 详细技术参数展开区域
                if (ImGui::CollapsingHeader("悬链线参数"))
                {
                    if (ImGui::BeginTable("DetailedTable",
                                          4,
                                          ImGuiTableFlags_Borders |
                                              ImGuiTableFlags_RowBg |
                                              ImGuiTableFlags_ScrollX))
                    {
                        ImGui::TableSetupColumn(
                            "ID",
                            ImGuiTableColumnFlags_WidthFixed,
                            40);
                        ImGui::TableSetupColumn(
                            "悬链线系数a",
                            ImGuiTableColumnFlags_WidthFixed,
                            120);
                        ImGui::TableSetupColumn(
                            "水平位移h",
                            ImGuiTableColumnFlags_WidthFixed,
                            120);
                        ImGui::TableSetupColumn(
                            "垂直位移k",
                            ImGuiTableColumnFlags_WidthFixed,
                            120);

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
                            ImGui::Text("%d", param.id);

                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%.6f", param.catenary_a);

                            ImGui::TableSetColumnIndex(2);
                            ImGui::Text("%.3f", param.catenary_h);

                            ImGui::TableSetColumnIndex(3);
                            ImGui::Text("%.3f", param.catenary_k);
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
                            "ID",
                            ImGuiTableColumnFlags_WidthFixed,
                            40);
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
                            ImGui::Text("%d", param.id);

                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%.3f", param.start_x);

                            ImGui::TableSetColumnIndex(2);
                            ImGui::Text("%.3f", param.start_y);

                            ImGui::TableSetColumnIndex(3);
                            ImGui::Text("%.3f", param.start_z);

                            ImGui::TableSetColumnIndex(4);
                            ImGui::Text("%.3f", param.end_x);

                            ImGui::TableSetColumnIndex(5);
                            ImGui::Text("%.3f", param.end_y);

                            ImGui::TableSetColumnIndex(6);
                            ImGui::Text("%.3f", param.end_z);
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
                WS_LOG_INFO("SQLite", "    start_x REAL,");
                WS_LOG_INFO("SQLite", "    start_y REAL,");
                WS_LOG_INFO("SQLite", "    start_z REAL,");
                WS_LOG_INFO("SQLite", "    end_x REAL,");
                WS_LOG_INFO("SQLite", "    end_y REAL,");
                WS_LOG_INFO("SQLite", "    end_z REAL,");
                WS_LOG_INFO("SQLite", "    length REAL,");
                WS_LOG_INFO("SQLite", "    width REAL,");
                WS_LOG_INFO("SQLite", "    max_sag REAL,");
                WS_LOG_INFO("SQLite", "    catenary_a REAL,");
                WS_LOG_INFO("SQLite", "    catenary_h REAL,");
                WS_LOG_INFO("SQLite", "    catenary_k REAL");
                WS_LOG_INFO("SQLite", ");");
                WS_LOG_INFO("SQLite", "");
                WS_LOG_INFO("SQLite", "-- 插入数据");

                for (const auto& param : World::powerLineParams)
                {
                    WS_LOG_INFO(
                        "SQLite",
                        "INSERT INTO power_lines (id, name, start_x, start_y, "
                        "start_z, end_x, end_y, end_z, "
                        "length, width, max_sag, catenary_a, catenary_h, "
                        "catenary_k) VALUES "
                        "({}, '{}', {:.3f}, {:.3f}, {:.3f}, {:.3f}, {:.3f}, "
                        "{:.3f}, "
                        "{:.2f}, {:.1f}, {:.3f}, {:.6f}, {:.3f}, {:.3f});",
                        param.id,
                        param.name,
                        param.start_x,
                        param.start_y,
                        param.start_z,
                        param.end_x,
                        param.end_y,
                        param.end_z,
                        param.length,
                        param.width,
                        param.max_sag,
                        param.catenary_a,
                        param.catenary_h,
                        param.catenary_k);
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
                                param.start_x,
                                param.start_y,
                                param.start_z,
                                param.end_x,
                                param.end_y,
                                param.end_z,
                                param.length,
                                param.width,
                                param.max_sag,
                                param.catenary_a,
                                param.catenary_h,
                                param.catenary_k);
                }
                WS_LOG_INFO("CSV", "=== CSV导出完成 ===");
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine();

            // 技术报告按钮
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.6f, 0.4f, 0.8f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.7f, 0.5f, 0.9f, 1.0f));
            if (ImGui::Button("技术报告", ImVec2(buttonWidth, 0)))
            {
                WS_LOG_INFO("Report", "=== 电力线路技术分析报告 ===");
                WS_LOG_INFO("Report", "处理文件: {}", World::lastProcessedFile);
                WS_LOG_INFO("Report",
                            "识别电力线数量: {}",
                            World::powerLineParams.size());

                float totalLength = 0.0f;
                float avgWidth    = 0.0f;
                float avgSag      = 0.0f;
                for (const auto& param : World::powerLineParams)
                {
                    totalLength += param.length;
                    avgWidth += param.width;
                    avgSag += param.max_sag;
                }
                if (!World::powerLineParams.empty())
                {
                    avgWidth /= World::powerLineParams.size();
                    avgSag /= World::powerLineParams.size();
                }

                WS_LOG_INFO("Report", "总线路长度: {:.2f}m", totalLength);
                WS_LOG_INFO("Report", "平均导线宽度: {:.1f}mm", avgWidth);
                WS_LOG_INFO("Report", "平均弧垂: {:.3f}m", avgSag);
                WS_LOG_INFO("Report", "=== 报告生成完成 ===");
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