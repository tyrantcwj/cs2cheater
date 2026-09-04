#pragma once
#include <cmath>
#include <cstring>
#include <cstdio>
#include <vector>
#include "memory/imemory.h"
#include "settings.h"

// Low-frequency radar HUD sync — only on map change / manual button / first in-match frame.
class RadarHudSync {
public:
    static constexpr float kRefRadarDiameter = 278.0f; // ~1080p in-game radar; capped on 4K
    static constexpr float kRefRadarMargin   = 21.0f;

    struct Layout {
        float size = BASE_RADAR_SIZE;
        float x    = BASE_RADAR_POS;
        float y    = BASE_RADAR_POS;
        float zoom = 0.35f;
    };

    struct ConvarResult {
        float value = 0.0f;
        bool  found = false;
    };

    bool try_sync_on_map_change(int screen_w, int screen_h) {
        return sync_once(screen_w, screen_h, "自动");
    }

    void force_sync(int screen_w, int screen_h) {
        sync_once(screen_w, screen_h, "手动");
    }

    bool try_sync_first_in_match(int screen_w, int screen_h, bool in_match) {
        if (!in_match) {
            synced_in_match_ = false;
            return false;
        }
        if (synced_in_match_) return false;
        synced_in_match_ = true;
        return sync_once(screen_w, screen_h, "进局");
    }

    void reset_session() { synced_in_match_ = false; }

    static Layout compute_layout(float screen_w, float screen_h,
                                 float hud_scale, float radar_scale) {
        (void)screen_w;
        float res = screen_h / REFERENCE_HEIGHT;
        if (res < 1.0f) res = 1.0f;
        if (res > 1.0f) res = 1.0f;

        Layout out{};
        out.size = kRefRadarDiameter * res * hud_scale;
        out.x    = kRefRadarMargin * res;
        out.y    = kRefRadarMargin * res;
        out.zoom = std::clamp(radar_scale, 0.25f, 1.0f);
        return out;
    }

private:
    bool synced_in_match_ = false;

    bool sync_once(int screen_w, int screen_h, const char* reason) {
        if (!g_memory) return false;

        const ConvarResult hud   = find_convar_once("cl_hud_radar_scale", 0.8f, 1.3f, 1.0f);
        const ConvarResult radar   = find_convar_once("cl_radar_scale",       0.25f, 1.0f, 0.35f);
        const float hud_scale   = hud.found ? hud.value : 1.0f;
        const float radar_scale = radar.found ? radar.value : g_settings.radar_zoom;

        const Layout layout = compute_layout(static_cast<float>(screen_w),
                                             static_cast<float>(screen_h),
                                             hud_scale, radar_scale);

        g_settings.radar_size = layout.size;
        g_settings.radar_x    = layout.x;
        g_settings.radar_y    = layout.y;
        if (radar.found)
            g_settings.radar_zoom = layout.zoom;

        printf("[+] 雷达%s对齐 size=%.0f pos=(%.0f,%.0f) zoom=%.2f hud=%.2f(%s) radar=%.2f(%s) 屏幕=%dx%d\n",
               reason, layout.size, layout.x, layout.y, g_settings.radar_zoom,
               hud_scale, hud.found ? "ok" : "默认",
               radar_scale, radar.found ? "ok" : "保留",
               screen_w, screen_h);
        return true;
    }

    static ConvarResult find_convar_once(const char* name, float min_v, float max_v, float fallback) {
        ConvarResult out{ fallback, false };
        if (!g_memory || !name || !*name) return out;

        const struct Mod { std::uintptr_t base; std::size_t size; } mods[] = {
            { g_memory->get_modules().tier0,  g_memory->get_modules().tier0_size },
            { g_memory->get_modules().client, g_memory->get_modules().client_size },
        };

        for (const auto& mod : mods) {
            if (!mod.base || !mod.size) continue;

            const std::uintptr_t name_addr = find_ascii_string(mod.base, mod.size, name);
            if (!name_addr) continue;

            const std::uintptr_t ref = g_memory->find_qword_in_sections(
                mod.base, name_addr, IMAGE_SCN_MEM_READ);
            if (!ref) continue;

            static constexpr int kNameOffsets[] = { 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58 };
            static constexpr int kValOffsets[]  = { 0x38, 0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58 };

            for (int name_off : kNameOffsets) {
                if (ref < static_cast<std::uintptr_t>(name_off)) continue;
                const std::uintptr_t cvar = ref - static_cast<std::uintptr_t>(name_off);

                for (int val_off : kValOffsets) {
                    const float v = g_memory->read<float>(cvar + val_off);
                    if (v >= min_v && v <= max_v) {
                        out.value = v;
                        out.found = true;
                        return out;
                    }
                }
            }
        }
        return out;
    }

    static std::uintptr_t find_ascii_string(std::uintptr_t base, std::size_t size,
                                            const char* needle) {
        if (!base || !needle || !*needle) return 0;
        const std::size_t nlen = std::strlen(needle) + 1;
        std::vector<std::uint8_t> chunk(0x10000);

        for (std::size_t off = 0; off < size; off += 0x10000) {
            const std::size_t read_sz = std::min<std::size_t>(0x10000, size - off);
            if (!g_memory->read_raw(base + off, chunk.data(), read_sz))
                continue;
            for (std::size_t i = 0; i + nlen <= read_sz; ++i) {
                if (std::memcmp(chunk.data() + i, needle, nlen) == 0)
                    return base + off + i;
            }
        }
        return 0;
    }
};

inline RadarHudSync g_radar_hud;
