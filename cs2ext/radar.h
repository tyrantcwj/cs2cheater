#pragma once
#include <imgui.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include "types.h"
#include "settings.h"
#include "overlay.h"

class Radar {
public:
    void draw(ImDrawList* draw, const RadarPlayer* players, int count,
              float local_x, float local_y, float local_yaw, int local_team,
              float map_scale, int sw, int sh) {

        if (!g_settings.draw_radar || !g_settings.master_switch) return;

        float size = g_settings.radar_size;

        // range = 210 * map_scale / zoom
        float range = (210.0f * map_scale) / g_settings.radar_zoom;

        float rx = g_settings.radar_x;
        float ry = g_settings.radar_y;
        float cx = rx + size * 0.5f;
        float cy = ry + size * 0.5f;
        float half = size * 0.5f;
        float alpha = g_settings.radar_bg_alpha;
        ImU32 bg = IM_COL32(15, 15, 15, (int)(alpha * 255));
        ImU32 border = IM_COL32(80, 80, 80, (int)(alpha * 255));

        if (g_settings.radar_circle) {
            draw->AddCircleFilled({cx, cy}, half, bg, 48);
            draw->AddCircle({cx, cy}, half, border, 48, 1.5f);
        } else {
            draw->AddRectFilled({rx, ry}, {rx + size, ry + size}, bg, 4.0f);
            draw->AddRect({rx, ry}, {rx + size, ry + size}, border, 4.0f, 0, 1.5f);
        }

        ImU32 cross_col = IM_COL32(255, 255, 255, (int)(alpha * 50));
        draw->AddLine({cx - 5, cy}, {cx + 5, cy}, cross_col);
        draw->AddLine({cx, cy - 5}, {cx, cy + 5}, cross_col);

        if (g_settings.radar_rings) {
            ImU32 ring_col = IM_COL32(255, 255, 255, (int)(alpha * 25));
            for (float f : {0.25f, 0.5f, 0.75f, 1.0f})
                draw->AddCircle({cx, cy}, half * f, ring_col, 32, 0.5f);
        }

        if (g_settings.radar_circle)
            draw->PushClipRect({rx, ry}, {rx + size, ry + size}, true);

        float yaw_rad = (local_yaw + 90.0f) * 3.14159265f / 180.0f;
        float cos_y = cosf(yaw_rad);
        float sin_y = sinf(yaw_rad);
        float scale = half / range;

        for (int i = 0; i < count; i++) {
            if (!players[i].valid || players[i].health <= 0) continue;

            bool enemy = (players[i].team != local_team);
            if (players[i].is_spotted) continue;
            if (!enemy && !g_settings.draw_teammates) continue;

            float dx = players[i].x - local_x;
            float dy = players[i].y - local_y;

            float rot_x, rot_y;
            if (g_settings.radar_rotate) {
                rot_x = dx * cos_y + dy * sin_y;
                rot_y = -dx * sin_y + dy * cos_y;
            } else {
                rot_x = dx;
                rot_y = -dy;
            }

            float px = cx + rot_x * scale;
            float py = cy - rot_y * scale;

            float off_x = px - cx, off_y = py - cy;
            float dist_sq = off_x * off_x + off_y * off_y;
            float max_r = half - 4.0f;

            if (g_settings.radar_circle) {
                if (dist_sq > max_r * max_r) {
                    float d = sqrtf(dist_sq);
                    px = cx + off_x / d * max_r;
                    py = cy + off_y / d * max_r;
                }
            } else {
                px = std::clamp(px, rx + 4.0f, rx + size - 4.0f);
                py = std::clamp(py, ry + 4.0f, ry + size - 4.0f);
            }

            // Use independent radar colors
            ImU32 col = enemy
                            ? float4_to_col(g_settings.radar_enemy_color)
                            : float4_to_col(g_settings.radar_team_color);

            float world_dist = sqrtf(dx * dx + dy * dy);
            float dot_r = std::clamp(5.0f - world_dist / range * 2.0f, 2.5f, 5.0f);

            draw->AddCircleFilled({px + 1, py + 1}, dot_r, IM_COL32(0, 0, 0, 80), 12);
            draw->AddCircleFilled({px, py}, dot_r, col, 12);
            draw->AddCircle({px, py}, dot_r, IM_COL32(0, 0, 0, 100), 12, 1.0f);

            if (g_settings.radar_names && players[i].name[0]) {
                ImFont* rfont = ImGui::GetFont();
                float   rfs   = g_settings.radar_names_font_size;
                ImVec2 ts = rfont->CalcTextSizeA(rfs, FLT_MAX, 0.0f, players[i].name);
                float tx = px - ts.x * 0.5f;
                float ty = py - dot_r - ts.y - 1;
                tx = std::clamp(tx, rx + 2, rx + size - ts.x - 2);
                ty = std::clamp(ty, ry + 2, ry + size - ts.y - 2);
                draw->AddText(rfont, rfs, {tx + 1, ty + 1}, IM_COL32(0, 0, 0, 160), players[i].name);
                draw->AddText(rfont, rfs, {tx, ty},         IM_COL32(255, 255, 255, 180), players[i].name);
            }
        }

        if (g_settings.radar_circle)
            draw->PopClipRect();

        draw_place_list(draw, players, count, local_team, rx, ry, size, sh);
    }

private:
    void draw_place_list(ImDrawList* draw, const RadarPlayer* players, int count,
                         int local_team, float rx, float ry, float size, int sh)
    {
        if (!g_settings.radar_place_list) return;

        struct Item {
            int meters;
            int health;
            bool enemy;
            const char* place;
            const char* name;
        };
        Item items[64];
        int n = 0;
        for (int i = 0; i < count && n < 64; i++) {
            if (!players[i].valid || players[i].health <= 0) continue;
            bool enemy = (players[i].team != local_team);
            if (!enemy && !g_settings.draw_teammates) continue;
            items[n++] = {
                players[i].meters,
                players[i].health,
                enemy,
                players[i].place[0] ? players[i].place : "未知",
                players[i].name[0] ? players[i].name : "-"
            };
        }
        if (n == 0) return;

        std::sort(items, items + n, [](const Item& a, const Item& b) {
            return a.meters < b.meters;
        });

        ImFont* font = g_overlay.menu_font ? g_overlay.menu_font : ImGui::GetFont();
        float fs = std::clamp(g_settings.radar_list_font_size, 8.0f, 28.0f);
        float pad_x = 8.0f;
        float pad_y = 5.0f;
        float row_h = fs + 5.0f;
        float header_h = fs + 3.0f;
        float col_gap = std::max(22.0f, fs * 1.4f);
        float dist_col_w = font->CalcTextSizeA(fs, FLT_MAX, 0.0f, "999m").x;
        float hp_col_w = font->CalcTextSizeA(fs, FLT_MAX, 0.0f, "100").x;
        float list_w = size;
        float min_list_w = pad_x * 2.0f + fs * 4.0f + fs * 3.0f + hp_col_w + col_gap + dist_col_w;
        if (list_w < min_list_w)
            list_w = min_list_w;
        float list_h = pad_y * 2.0f + header_h + row_h * n;
        float list_x = rx;
        float list_y = ry + size + 6.0f;
        if (list_y + list_h > (float)sh - 4.0f)
            list_y = (float)sh - list_h - 4.0f;

        draw->AddRectFilled({list_x, list_y}, {list_x + list_w, list_y + list_h},
                            IM_COL32(8, 10, 14, 190), 4.0f);
        draw->AddRect({list_x, list_y}, {list_x + list_w, list_y + list_h},
                      IM_COL32(70, 78, 88, 160), 4.0f, 0, 1.0f);

        auto outlined = [&](float x, float y, ImU32 col, const char* text) {
            draw->AddText(font, fs, {x + 1, y + 1}, IM_COL32(0, 0, 0, 180), text);
            draw->AddText(font, fs, {x, y}, col, text);
        };

        auto fit_text = [&](char* out, size_t outsz, const char* src, float max_w) {
            if (!src || !src[0]) { out[0] = 0; return; }
            if (font->CalcTextSizeA(fs, FLT_MAX, 0.0f, src).x <= max_w) {
                std::strncpy(out, src, outsz - 1);
                out[outsz - 1] = 0;
                return;
            }
            float dots_w = font->CalcTextSizeA(fs, FLT_MAX, 0.0f, "..").x;
            size_t best = 0;
            size_t len = std::strlen(src);
            for (size_t i = 1; i <= len && i < outsz - 3; i++) {
                char tmp[256];
                if (i >= sizeof(tmp)) break;
                std::memcpy(tmp, src, i);
                tmp[i] = 0;
                if (font->CalcTextSizeA(fs, FLT_MAX, 0.0f, tmp).x + dots_w > max_w)
                    break;
                best = i;
            }
            while (best > 0 && ((unsigned char)src[best] & 0xC0) == 0x80)
                best--;
            std::memcpy(out, src, best);
            std::memcpy(out + best, "..", 3);
        };

        float hx = list_x + pad_x;
        float hy = list_y + pad_y;
        float name_col_w = std::max(fs * 4.0f, list_w - pad_x * 2.0f - fs * 3.0f - hp_col_w - col_gap - dist_col_w);
        float place_x = list_x + pad_x + name_col_w;
        float hp_right = list_x + list_w - pad_x - dist_col_w - col_gap;
        ImU32 head_col = IM_COL32(180, 186, 196, 200);
        outlined(hx, hy, head_col, "游戏ID");
        outlined(place_x, hy, head_col, "位置");
        {
            const char* hp_h = "血量";
            ImVec2 ts = font->CalcTextSizeA(fs, FLT_MAX, 0.0f, hp_h);
            outlined(hp_right - ts.x, hy, head_col, hp_h);
        }
        {
            const char* dist_h = "距离";
            ImVec2 ts = font->CalcTextSizeA(fs, FLT_MAX, 0.0f, dist_h);
            outlined(list_x + list_w - pad_x - ts.x, hy, head_col, dist_h);
        }

        float y = hy + header_h;
        for (int i = 0; i < n; i++) {
            ImU32 col = items[i].enemy
                ? float4_to_col(g_settings.radar_enemy_color)
                : float4_to_col(g_settings.radar_team_color);

            char name_fit[96];
            fit_text(name_fit, sizeof(name_fit), items[i].name, name_col_w - 8.0f);
            outlined(hx, y, col, name_fit);

            char place_fit[48];
            float place_max_w = list_w - pad_x * 2.0f - name_col_w - dist_col_w - col_gap - hp_col_w;
            if (place_max_w < 36.0f) place_max_w = 36.0f;
            fit_text(place_fit, sizeof(place_fit), items[i].place, place_max_w);
            outlined(place_x, y, col, place_fit);

            char hp_txt[8];
            snprintf(hp_txt, sizeof(hp_txt), "%d", items[i].health);
            float hp_t = std::clamp(items[i].health / 100.0f, 0.0f, 1.0f);
            ImU32 hp_col = IM_COL32((int)(255 * (1.0f - hp_t)), (int)(220 * hp_t), 48, 255);
            ImVec2 hp_ts = font->CalcTextSizeA(fs, FLT_MAX, 0.0f, hp_txt);
            outlined(hp_right - hp_ts.x, y, hp_col, hp_txt);

            char dist[16];
            snprintf(dist, sizeof(dist), "%dm", items[i].meters);
            ImVec2 ts = font->CalcTextSizeA(fs, FLT_MAX, 0.0f, dist);
            outlined(list_x + list_w - pad_x - ts.x, y, col, dist);
            y += row_h;
        }
    }
};

inline Radar g_radar;