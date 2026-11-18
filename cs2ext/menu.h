#pragma once
#include <imgui.h>
#include <cfloat>
#include "settings.h"
#include "types.h"
#include "utils.h"
#include "crosshair.h"
#include "overlay.h"
#include "config.h"

class Menu {
public:
    void toggle() { g_settings.menu_open = !g_settings.menu_open; }

    void render() {
        if (!g_settings.menu_open) return;

        if (g_settings.menu_x >= 0 && g_settings.menu_y >= 0)
            ImGui::SetNextWindowPos({g_settings.menu_x, g_settings.menu_y}, ImGuiCond_FirstUseEver);
        const float ui = g_overlay.get_ui_scale();
        ImGui::SetNextWindowSize({580.0f * ui, 520.0f * ui}, ImGuiCond_FirstUseEver);

        ImGui::PushFont(g_overlay.menu_font);

        ImGui::Begin("##mainwindow", &g_settings.menu_open,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        ImVec2 pos = ImGui::GetWindowPos();
        g_settings.menu_x = pos.x;
        g_settings.menu_y = pos.y;

        render_header();

        ImGui::Spacing();

        if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_FittingPolicyScroll)) {
            if (ImGui::BeginTabItem("主页"))  { render_tab_main();  ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("透视"))  { render_tab_esp();   ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("雷达"))  { render_tab_radar(); ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("瞄准"))  { render_tab_aim();   ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("杂项"))  { render_tab_misc();  ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("外观"))  { render_tab_menu_style(); ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }

        ImGui::End();
        ImGui::PopFont();
    }

private:
    bool bind_waiting_menu = false;
    bool bind_waiting_master = false;
    bool bind_waiting_exit = false;
    bool reset_popup_open = false;
    bool bind_waiting_aimbot = false;
    bool bind_waiting_trigger = false;

    ImVec4 accent_color() const {
        return { g_settings.menu_accent_color[0], g_settings.menu_accent_color[1],
                 g_settings.menu_accent_color[2], g_settings.menu_accent_color[3] };
    }

    void render_header() {
        ImVec4 accent = accent_color();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 win_pos = ImGui::GetWindowPos();
        float win_w = ImGui::GetWindowWidth();
        const float ui = g_overlay.get_ui_scale();

        dl->AddRectFilled(
            { win_pos.x, win_pos.y },
            { win_pos.x + win_w, win_pos.y + 3.0f * ui },
            ImGui::ColorConvertFloat4ToU32({ accent.x, accent.y, accent.z, 0.95f }),
            10.0f * ui, ImDrawFlags_RoundCornersTop);

        ImGui::PushFont(g_overlay.menu_font);
        const char* title = "iTyc-cs2";
        const float title_size = ImGui::GetFontSize() * 1.15f;
        ImVec2 title_pos = ImGui::GetCursorScreenPos();
        dl->AddText(g_overlay.menu_font, title_size, title_pos,
                    ImGui::ColorConvertFloat4ToU32(accent), title);
        ImGui::Dummy({ g_overlay.menu_font->CalcTextSizeA(title_size, FLT_MAX, 0.0f, title).x,
                       title_size });
        ImGui::PopFont();

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 130.0f * ui);
        render_status_badge(g_settings.master_switch);
        ImGui::Dummy({ 0, 4.0f * ui });
        ImGui::Separator();
    }

    void render_status_badge(bool active) {
        const float ui = g_overlay.get_ui_scale();
        ImVec4 fg = active ? ImVec4{1.0f, 0.65f, 0.15f, 1.0f}
                           : ImVec4{0.95f, 0.35f, 0.35f, 1.0f};
        ImVec4 bg = active ? ImVec4{0.32f, 0.18f, 0.05f, 0.95f}
                           : ImVec4{0.28f, 0.10f, 0.10f, 0.95f};
        const char* label = active ? "运行中" : "已暂停";

        ImGui::PushStyleColor(ImGuiCol_Button, bg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bg);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, bg);
        ImGui::PushStyleColor(ImGuiCol_Text, fg);
        ImGui::Button(label, { 108.0f * ui, 28.0f * ui });
        ImGui::PopStyleColor(4);
    }

    void section_title(const char* title) {
        ImGui::Spacing();
        ImGui::TextColored(accent_color(), "%s", title);
        ImGui::Separator();
        ImGui::Spacing();
    }

    void begin_panel(const char* id) {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        ImGui::BeginChild(id, { 0, 0 }, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border,
                          ImGuiWindowFlags_None);
        ImGui::Spacing();
    }

    void end_panel() {
        ImGui::Spacing();
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }

    // RAII helper for panels
    struct PanelScope {
        Menu* m;
        PanelScope(Menu* menu, const char* id) : m(menu) { m->begin_panel(id); }
        ~PanelScope() { m->end_panel(); }
    };

    void render_tab_main() {
        section_title("运行状态");

        { PanelScope p(this, "##main_status");
            ImGui::Text("总开关");
            ImGui::SameLine(120);
            if (g_settings.master_switch)
                ImGui::TextColored({1.0f, 0.65f, 0.15f, 1}, "已启用  (%s)", vk_name(g_settings.key_master));
            else
                ImGui::TextColored({1.0f, 0.3f, 0.3f, 1}, "已禁用  (%s)", vk_name(g_settings.key_master));
        }

        section_title("快捷键");
        { PanelScope p(this, "##main_keys");
            render_key_bind("菜单开关", g_settings.key_menu, bind_waiting_menu);
            render_key_bind("总开关", g_settings.key_master, bind_waiting_master);
            render_key_bind("退出程序", g_settings.key_exit, bind_waiting_exit);
        }

        section_title("性能与后端");
        begin_panel("##main_perf");
        ImGui::Checkbox("垂直同步", &g_settings.use_vsync);
        ImGui::BeginDisabled(g_settings.use_vsync);
        ImGui::SliderFloat("目标帧率", &g_settings.target_fps, 30, 1000, "%.0f FPS");
        ImGui::EndDisabled();
        ImGui::TextColored({0.5f, 0.5f, 0.5f, 1}, "帧率越高，透视绘制越平滑");

        ImGui::Spacing();
        ImGui::Text("内存读取后端");
        static bool backend_changed = false;
        backend_changed |= ImGui::RadioButton("WinAPI", &g_settings.memory_backend, 1);
        ImGui::SameLine();
        backend_changed |= ImGui::RadioButton("Syscall", &g_settings.memory_backend, 2);
        ImGui::SameLine();
        backend_changed |= ImGui::RadioButton("Kernel", &g_settings.memory_backend, 3);
        if (backend_changed)
            ImGui::TextColored(ImVec4(1, 0.45f, 0.25f, 1), "切换后端后需重启程序");
        end_panel();

        section_title("游戏识别");
        begin_panel("##main_game");
        ImGui::InputText("进程名", g_settings.target_process, sizeof(g_settings.target_process));
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("任务管理器「详细信息」里的 exe 名称，例如 cs2.exe");
        ImGui::InputText("窗口标题", g_settings.target_window, sizeof(g_settings.target_window));
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("游戏窗口标题，例如「反恐精英：全球攻势」。\n留空也会尝试按进程自动查找。");
        ImGui::TextColored({0.55f, 0.55f, 0.55f, 1}, "修改后需重启程序生效");
        end_panel();

        ImGui::Spacing();
        const float ui = g_overlay.get_ui_scale();
        if (ImGui::Button("重置所有设置", { ImGui::GetContentRegionAvail().x, 32.0f * ui }))
            reset_popup_open = true;
        if (reset_popup_open) {
            ImGui::OpenPopup("Reset?##confirm");
            reset_popup_open = false;
        }
        if (ImGui::BeginPopupModal("Reset?##confirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("是否将所有设置恢复默认？");
            ImGui::Separator();
            if (ImGui::Button("确认", {100.0f * ui, 0})) {
                g_settings.reset();
                g_settings.apply_startup_adapt(
                    static_cast<float>(g_overlay.width),
                    static_cast<float>(g_overlay.height));
                g_overlay.font_rebuild_needed = true;
                g_overlay.apply_menu_style();
                Config::save("cs2esp.ini");
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("取消", {100, 0})) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }

    void render_tab_esp() {
        section_title("总开关");
        begin_panel("##esp_main");
        ImGui::Checkbox("启用透视", &g_settings.esp_enabled);
        end_panel();

        ImVec4 theme_accent = {g_settings.esp_theme_color[0], g_settings.esp_theme_color[1],
                               g_settings.esp_theme_color[2], g_settings.esp_theme_color[3]};
        ImGui::TextColored(theme_accent, "透视主题色");
        ImGui::Checkbox("使用主题色##esptheme", &g_settings.esp_use_theme);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(
                "开启后：方框等颜色会从主题色自动推导。\n"
                "敌人用原色，队友偏蓝。开启时下方单独颜色设置无效。");

        if (g_settings.esp_use_theme) {
            ImGui::Indent();
            ImGui::ColorEdit4("主题色##tc", g_settings.esp_theme_color,
                              ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

            // Quick presets
            ImGui::Text("预设:");
            struct Preset { const char* name; float r, g, b; };
            static constexpr Preset presets[] = {
                {"橙色",       1.00f, 0.55f, 0.00f},
                {"亮绿",       0.20f, 1.00f, 0.30f},
                {"蓝色",       0.235f,0.68f, 0.93f},
                {"洋红",    0.90f, 0.10f, 0.80f},
                {"白色",      0.95f, 0.95f, 0.95f},
                {"红色",        1.00f, 0.15f, 0.15f},
            };
            for (const auto& pr : presets) {
                if (ImGui::Button(pr.name, {65, 0})) {
                    g_settings.esp_theme_color[0] = pr.r;
                    g_settings.esp_theme_color[1] = pr.g;
                    g_settings.esp_theme_color[2] = pr.b;
                    g_settings.esp_theme_color[3] = 1.0f;
                }
                ImGui::SameLine();
            }
            ImGui::NewLine();
            ImGui::Unindent();

            // While theme is active, gray out individual color pickers with a note
            ImGui::TextColored({0.5f, 0.5f, 0.5f, 1}, "(下方单独颜色已被主题覆盖)");
            ImGui::Separator();
        } else {
            ImGui::Separator();
        }

        // ================================================================
        //  CHAMS / SKELETON
        // ================================================================
        ImGui::Text("高亮样式:");
        ImGui::RadioButton("填充",   &g_settings.chams_style, 0); ImGui::SameLine();
        ImGui::RadioButton("线框",     &g_settings.chams_style, 1); ImGui::SameLine();
        ImGui::RadioButton("发光",     &g_settings.chams_style, 2); ImGui::SameLine();
        ImGui::RadioButton("骨骼", &g_settings.chams_style, 3);

        ImGui::Checkbox("绘制头部", &g_settings.draw_head);
        ImGui::Checkbox("显示队友", &g_settings.draw_teammates);
        ImGui::Separator();

        // ================================================================
        // DISTANCE OPACITY
        // ================================================================
        ImGui::Checkbox("距离透明度衰减", &g_settings.esp_opacity_drop);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("随距离淡出透视元素。\n不影响雷达和观战列表。");
        if (g_settings.esp_opacity_drop) {
            ImGui::Indent();
            ImGui::SliderFloat("开始淡出##opd", &g_settings.esp_opacity_drop_start,
                               100.0f, 3000.0f, "%.0f 单位");
            ImGui::SliderFloat("完全淡出##opd",   &g_settings.esp_opacity_drop_end,
                               200.0f, 5000.0f, "%.0f 单位");
            ImGui::SliderFloat("最小透明度##opd", &g_settings.esp_opacity_drop_min,
                               0.0f, 1.0f, "%.2f");
            // Clamp: start must be less than end
            if (g_settings.esp_opacity_drop_start >= g_settings.esp_opacity_drop_end)
                g_settings.esp_opacity_drop_start = g_settings.esp_opacity_drop_end - 100.0f;
            ImGui::Unindent();
        }
        ImGui::Separator();

        // ================================================================
        //  BOX
        // ================================================================
        ImGui::Checkbox("方框", &g_settings.draw_box);
        if (g_settings.draw_box) {
            ImGui::Indent();
            ImGui::RadioButton("角框##bs", &g_settings.box_style, 0); ImGui::SameLine();
            ImGui::RadioButton("全框##bs",    &g_settings.box_style, 1); ImGui::SameLine();
            ImGui::RadioButton("虚线##bs",  &g_settings.box_style, 2);
            ImGui::SliderFloat("线宽##bt", &g_settings.box_thickness,  0.5f, 4.0f, "%.1f");
            ImGui::SliderFloat("横向边距",     &g_settings.box_padding_x,  0.0f, 20.0f, "%.1f");
            ImGui::SliderFloat("纵向边距",     &g_settings.box_padding_y,  0.0f, 20.0f, "%.1f");
            if (g_settings.box_style == 0)
                ImGui::SliderFloat("角框比例", &g_settings.box_corner_pct, 0.1f, 0.5f, "%.2f");
            ImGui::Unindent();
        }
        ImGui::Checkbox("名字", &g_settings.draw_name);
        ImGui::SameLine();
        ImGui::Checkbox("位置", &g_settings.draw_place);
        ImGui::SameLine();
        ImGui::Checkbox("血条", &g_settings.draw_healthbar);
        ImGui::SameLine();
        ImGui::Checkbox("血量", &g_settings.draw_health_text);
        ImGui::Separator();

        // ================================================================
        //  ESP FONT
        // ================================================================
        ImGui::Text("透视字体:");
        render_esp_font_selector();
        ImGui::Separator();

        // ================================================================
        //  BODY TUNING
        // ================================================================
        ImGui::Text("身体绘制");
        ImGui::SliderFloat("身体宽度",  &g_settings.body_width_scale, 0.3f, 3.0f,  "%.2f");
        ImGui::SliderFloat("头部大小",   &g_settings.head_radius,      1,    10,     "%.1f");
        ImGui::SliderFloat("深度缩放", &g_settings.depth_scale,      100,  1500,   "%.0f");
        ImGui::SliderFloat("外发光",  &g_settings.glow_expand_outer, 0,   15,     "%.1f");
        ImGui::SliderFloat("内发光",  &g_settings.glow_expand_inner, 0,   10,     "%.1f");
        if (ImGui::Button("重置身体参数")) {
            g_settings.body_width_scale  = 1.0f;
            g_settings.head_radius       = 6.0f;
            g_settings.depth_scale       = 500.0f;
            g_settings.glow_expand_outer = 6.0f;
            g_settings.glow_expand_inner = 3.0f;
        }
        ImGui::Separator();

        // ================================================================
        //  ESP COLORS  (only shown when theme is OFF)
        // ================================================================
        if (!g_settings.esp_use_theme) {
            ImGui::Text("透视颜色");
            ImGui::Columns(2, nullptr, false);
            ImGui::Text("敌人");
            ImGui::ColorEdit4("填充##ef",    g_settings.enemy_fill,    ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("外框##eo", g_settings.enemy_outline, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("发光##eg",    g_settings.enemy_glow,    ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
            ImGui::NextColumn();
            ImGui::Text("队友");
            ImGui::ColorEdit4("填充##tf",    g_settings.team_fill,    ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("外框##to", g_settings.team_outline, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("发光##tg",    g_settings.team_glow,    ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
            ImGui::Columns(1);
        }
    }

    void render_tab_radar() {
        section_title("雷达开关");
        begin_panel("##radar_toggle");
        ImGui::Checkbox("显示雷达", &g_settings.draw_radar);
        end_panel();

        section_title("显示选项");
        begin_panel("##radar_opts");
        ImGui::Checkbox("圆形", &g_settings.radar_circle);
        ImGui::Checkbox("死亡/Tab切换方形", &g_settings.radar_auto_square);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("启用后，玩家死亡或按住 Tab 时，雷达自动切换为方形全图以匹配游戏小地图。");
        ImGui::Checkbox("随视角旋转", &g_settings.radar_rotate);
        ImGui::Checkbox("距离环", &g_settings.radar_rings);
        ImGui::Checkbox("玩家名字", &g_settings.radar_names);
        ImGui::Checkbox("地点列表", &g_settings.radar_place_list);
        if (g_settings.radar_names) {
            if (ImGui::SliderFloat("名字字体##rnf", &g_settings.radar_names_font_size, 8.0f, 20.0f, "%.0f"))
                g_overlay.font_rebuild_needed = true;
        }
        if (g_settings.radar_place_list)
            ImGui::SliderFloat("列表字体##rlf", &g_settings.radar_list_font_size, 8.0f, 28.0f, "%.0f");
        end_panel();

        section_title("布局");
        begin_panel("##radar_layout");
        ImGui::SliderFloat("大小", &g_settings.radar_size, 100, 2400, "%.0f");
        ImGui::SliderFloat("缩放", &g_settings.radar_zoom, 0.1f, 1.0f, "%.2fx");
        ImGui::SliderFloat("透明度", &g_settings.radar_bg_alpha, 0.0f, 1.0f, "%.2f");
        ImGui::DragFloat("位置 X", &g_settings.radar_x, 1, 0, 3000);
        ImGui::DragFloat("位置 Y", &g_settings.radar_y, 1, 0, 2000);
        end_panel();

        section_title("颜色");
        begin_panel("##radar_colors");
        ImGui::ColorEdit4("敌人##re", g_settings.radar_enemy_color,
                          ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
        ImGui::ColorEdit4("队友##rt", g_settings.radar_team_color,
                          ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
        end_panel();
    }

    void render_tab_aim() {
        begin_panel("##aim_warn");
        ImGui::TextColored({1.0f, 0.45f, 0.25f, 1}, "警告：瞄准辅助存在封号风险，请谨慎使用。");
        end_panel();

        section_title("自瞄");
        begin_panel("##aimbot");
        ImGui::Checkbox("启用自瞄", &g_settings.aimbot_enabled);
        if (g_settings.aimbot_enabled) {
            render_key_bind("按键", g_settings.key_aimbot, bind_waiting_aimbot);
            ImGui::SliderInt("视野范围", &g_settings.aimbot_fov, 1, 360);
            ImGui::SliderFloat("平滑度", &g_settings.aimbot_smooth, 1.0f, 20.0f, "%.1f");
            ImGui::Checkbox("仅瞄头", &g_settings.aimbot_head_only);
        }
        end_panel();

        section_title("扳机");
        begin_panel("##trigger");
        ImGui::Checkbox("启用扳机", &g_settings.triggerbot_enabled);
        if (g_settings.triggerbot_enabled) {
            render_key_bind("扳机键", g_settings.key_triggerbot, bind_waiting_trigger);
            ImGui::SliderInt("延迟(毫秒)", &g_settings.triggerbot_delay, 0, 300);
            ImGui::Checkbox("仅打头##trigger", &g_settings.triggerbot_head_only);
        }
        end_panel();
    }

    void render_tab_misc() {
        section_title("观战者");
        begin_panel("##spec");
        ImGui::Checkbox("显示观战者", &g_settings.draw_spectators);
        if (g_settings.draw_spectators) {
            ImGui::DragFloat("列表 X##sx", &g_settings.spec_x, 1, -1, 3000, "%.0f");
            ImGui::SameLine();
            if (ImGui::Button("自动##specauto")) g_settings.spec_x = -1.0f;
            ImGui::DragFloat("列表 Y##sy", &g_settings.spec_y, 1, 0, 2000, "%.0f");
        }
        end_panel();

        section_title("准星");
        begin_panel("##xhair");
        ImGui::Checkbox("启用##xhair", &g_settings.crosshair_enabled);
        if (g_settings.crosshair_enabled) {
            ImGui::Text("形状:");
            ImGui::RadioButton("+##xs", &g_settings.crosshair_shape, 0); ImGui::SameLine();
            ImGui::RadioButton("T##xs", &g_settings.crosshair_shape, 1); ImGui::SameLine();
            ImGui::RadioButton("O##xs", &g_settings.crosshair_shape, 2); ImGui::SameLine();
            ImGui::RadioButton("Dot##xs", &g_settings.crosshair_shape, 3); ImGui::SameLine();
            ImGui::RadioButton("+O##xs", &g_settings.crosshair_shape, 4); ImGui::SameLine();

            ImGui::ColorEdit4("颜色##xcol", g_settings.crosshair_color, ImGuiColorEditFlags_NoInputs);
            ImGui::SliderFloat("大小##xsz", &g_settings.crosshair_size, 0.5f, 20, "%.1f");
            ImGui::SliderFloat("粗细##xth", &g_settings.crosshair_thickness, 0.5f, 5, "%.1f");

            bool has_gap = g_settings.crosshair_shape <= 1 ||
                           g_settings.crosshair_shape == 4 ||
                           g_settings.crosshair_shape == 6;
            if (has_gap)
                ImGui::SliderFloat("间隙##xgap", &g_settings.crosshair_gap, -10, 10, "%.1f");

            ImGui::Checkbox("描边##xol", &g_settings.crosshair_outline);
            if (g_settings.crosshair_outline) {
                ImGui::ColorEdit4("描边颜色##xolc", g_settings.crosshair_outline_color, ImGuiColorEditFlags_NoInputs);
                ImGui::SliderFloat("描边宽度##xolt", &g_settings.crosshair_outline_thickness, 1, 3, "%.0f");
            }
            if (g_settings.crosshair_shape != 3) {
                ImGui::Checkbox("中心点##xdot", &g_settings.crosshair_dot);
                if (g_settings.crosshair_dot)
                    ImGui::SliderFloat("点大小##xds", &g_settings.crosshair_dot_size, 1, 4, "%.0f");
            }

            ImGui::Separator();
            ImGui::Text("预览");
            ImVec2 pp = ImGui::GetCursorScreenPos();
            float psz = 60;
            ImGui::InvisibleButton("##xprev", {psz, psz});
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(pp, {pp.x + psz, pp.y + psz}, IM_COL32(20, 20, 20, 255));
            dl->AddRect(pp, {pp.x + psz, pp.y + psz}, IM_COL32(50, 50, 50, 255));
            dl->PushClipRect(pp, {pp.x + psz, pp.y + psz}, true);
            ImDrawListFlags old = dl->Flags;
            dl->Flags &= ~ImDrawListFlags_AntiAliasedLines;
            dl->Flags &= ~ImDrawListFlags_AntiAliasedFill;
            Crosshair::Config prev_cfg = {
                true, g_settings.crosshair_shape, g_settings.crosshair_size,
                g_settings.crosshair_gap, g_settings.crosshair_thickness,
                float4_to_col(g_settings.crosshair_color),
                g_settings.crosshair_outline, g_settings.crosshair_outline_thickness,
                float4_to_col(g_settings.crosshair_outline_color),
                g_settings.crosshair_dot, g_settings.crosshair_dot_size,
            };
            float pcx = floorf(pp.x + psz * 0.5f) + 0.5f;
            float pcy = floorf(pp.y + psz * 0.5f) + 0.5f;
            g_crosshair.draw_preview(dl, pcx, pcy, prev_cfg);
            dl->Flags = old;
            dl->PopClipRect();
        }
        end_panel();
    }

    void render_tab_menu_style() {
        section_title("主题");
        begin_panel("##style_theme");
        bool style_changed = false;
        style_changed |= ImGui::ColorEdit4("强调色", g_settings.menu_accent_color,
                                            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        style_changed |= ImGui::ColorEdit4("边框颜色", g_settings.menu_border_color,
                                            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        style_changed |= ImGui::SliderFloat("背景透明度", &g_settings.menu_bg_alpha, 0.3f, 1.0f, "%.2f");
        end_panel();

        section_title("预设主题");
        begin_panel("##style_presets");
        if (ImGui::Button("赛博橙", {120, 0})) {
            float cc[] = {1, 0.55f, 0, 1}; float b[] = {1, 0.55f, 0, 0.3f};
            memcpy(g_settings.menu_accent_color, cc, 16);
            memcpy(g_settings.menu_border_color, b, 16);
            g_settings.menu_bg_alpha = 0.92f; style_changed = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("血红", {120, 0})) {
            float cc[] = {1, 0.2f, 0.15f, 1}; float b[] = {1, 0.2f, 0.15f, 0.3f};
            memcpy(g_settings.menu_accent_color, cc, 16);
            memcpy(g_settings.menu_border_color, b, 16);
            g_settings.menu_bg_alpha = 0.92f; style_changed = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("电光蓝", {120, 0})) {
            float cc[] = {0.2f, 0.5f, 1, 1}; float b[] = {0.2f, 0.5f, 1, 0.3f};
            memcpy(g_settings.menu_accent_color, cc, 16);
            memcpy(g_settings.menu_border_color, b, 16);
            g_settings.menu_bg_alpha = 0.92f; style_changed = true;
        }
        if (ImGui::Button("紫雾", {120, 0})) {
            float cc[] = {0.7f, 0.3f, 1, 1}; float b[] = {0.7f, 0.3f, 1, 0.3f};
            memcpy(g_settings.menu_accent_color, cc, 16);
            memcpy(g_settings.menu_border_color, b, 16);
            g_settings.menu_bg_alpha = 0.90f; style_changed = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("金色", {120, 0})) {
            float cc[] = {1, 0.8f, 0.2f, 1}; float b[] = {1, 0.8f, 0.2f, 0.3f};
            memcpy(g_settings.menu_accent_color, cc, 16);
            memcpy(g_settings.menu_border_color, b, 16);
            g_settings.menu_bg_alpha = 0.92f; style_changed = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("极简", {120, 0})) {
            float cc[] = {0.7f, 0.7f, 0.7f, 1}; float b[] = {0.4f, 0.4f, 0.4f, 0.2f};
            memcpy(g_settings.menu_accent_color, cc, 16);
            memcpy(g_settings.menu_border_color, b, 16);
            g_settings.menu_bg_alpha = 0.88f; style_changed = true;
        }

        if (style_changed)
            g_overlay.apply_menu_style();
        end_panel();

        section_title("字体");
        begin_panel("##style_font");
        render_menu_font_selector();
        if (ImGui::SliderFloat("菜单字体大小", &g_settings.menu_font_size, 10.0f, 36.0f, "%.0f"))
            g_overlay.font_rebuild_needed = true;
        end_panel();
    }

    void render_key_bind(const char* label, int& key, bool& waiting, bool allow_mouse1 = false) {
        const float ui = g_overlay.get_ui_scale();
        ImGui::Text("%s:", label);
        ImGui::SameLine(150.0f * ui);

        char btn[64];
        if (waiting)
            snprintf(btn, sizeof(btn), "[...]##%s", label);
        else
            snprintf(btn, sizeof(btn), "%s##%s", vk_name(key), label);

        if (ImGui::Button(btn, {132.0f * ui, 0})) {
            waiting = true;
        }

        if (waiting) {
            int pressed = scan_any_key(allow_mouse1);
            if (pressed > 0)   { key = pressed; waiting = false; }
            else if (pressed == -1) { waiting = false; } // Escape = cancel
        }
    }

    void render_esp_font_selector() {
        auto& fonts = g_overlay.available_fonts;
        if (fonts.empty()) { ImGui::Text("无可用字体"); return; }
        const char* preview = (g_settings.esp_font_index >= 0 &&
                               g_settings.esp_font_index < (int)fonts.size())
                                  ? fonts[g_settings.esp_font_index].display_name.c_str() : "?";
        if (ImGui::BeginCombo("##espfont", preview)) {
            for (int i = 0; i < (int)fonts.size(); i++) {
                bool sel = (g_settings.esp_font_index == i);
                if (ImGui::Selectable(fonts[i].display_name.c_str(), sel)) {
                    if (g_settings.esp_font_index != i) {
                        g_settings.esp_font_index = i;
                        g_overlay.font_rebuild_needed = true;
                    }
                }
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (g_overlay.esp_font) {
            ImGui::PushFont(g_overlay.esp_font);
            float sz = g_settings.name_font_size;
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            const char* sample = "Player_Name 123";
            ImVec2 ts = g_overlay.esp_font->CalcTextSizeA(sz, FLT_MAX, 0, sample);
            dl->AddRectFilled(p, {p.x + ts.x + 8, p.y + ts.y + 4}, IM_COL32(15, 15, 15, 200), 3);
            dl->AddText(g_overlay.esp_font, sz, {p.x + 4, p.y + 2},
                        float4_to_col(g_settings.name_color), sample);
            ImGui::Dummy({ts.x + 8, ts.y + 6});
            ImGui::PopFont();
        }
    }

    void render_menu_font_selector() {
        auto& fonts = g_overlay.menu_fonts;
        if (fonts.empty()) { ImGui::Text("无可用字体"); return; }
        const char* preview = (g_settings.menu_font_index >= 0 &&
                               g_settings.menu_font_index < (int)fonts.size())
                                  ? fonts[g_settings.menu_font_index].display_name.c_str() : "?";
        if (ImGui::BeginCombo("##menufont", preview)) {
            for (int i = 0; i < (int)fonts.size(); i++) {
                bool sel = (g_settings.menu_font_index == i);
                if (ImGui::Selectable(fonts[i].display_name.c_str(), sel)) {
                    if (g_settings.menu_font_index != i) {
                        g_settings.menu_font_index = i;
                        g_overlay.font_rebuild_needed = true;
                    }
                }
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
};

inline Menu g_menu;