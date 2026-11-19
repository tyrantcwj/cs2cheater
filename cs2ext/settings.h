#pragma once

#include <cmath>

static constexpr float REFERENCE_WIDTH  = 1920.0f;
static constexpr float REFERENCE_HEIGHT = 1080.0f;

static constexpr float BASE_RADAR_SIZE            = 498.0f;
static constexpr float BASE_RADAR_POS             = 50.0f;
static constexpr float BASE_RADAR_NAMES_FONT_SIZE = 11.0f;
static constexpr float BASE_RADAR_LIST_FONT_SIZE  = 14.0f;
static constexpr float BASE_MENU_FONT_SIZE        = 20.0f;
static constexpr float BASE_SPEC_Y                = 30.0f;

inline float resolution_scale(float screen_w, float screen_h) {
    float sw = screen_w / REFERENCE_WIDTH;
    float sh = screen_h / REFERENCE_HEIGHT;
    float s  = (sw < sh) ? sw : sh;
    if (s < 1.0f) s = 1.0f;
    if (s > 3.0f) s = 3.0f;
    return s;
}

// Menu layout scale — gentler than full resolution ratio to avoid oversized UI on 4K.
inline float menu_ui_scale(float screen_w, float screen_h) {
    float raw = resolution_scale(screen_w, screen_h);
    if (raw <= 1.01f) return 1.0f;
    float s = 1.0f + (raw - 1.0f) * 0.35f;
    return (s > 1.35f) ? 1.35f : s;
}

inline bool g_config_loaded_from_disk = false;

struct MenuSettings {
    int memory_backend = -1;

    char target_process[64] = "cs2.exe";
    char target_window[128] = "反恐精英：全球攻势";

    bool master_switch = true;
    bool esp_enabled = true;
    bool draw_box = true;
    bool draw_healthbar = false;
    bool draw_health_text = false;
    bool draw_name = true;
    bool draw_place = true;
    bool draw_weapon = false;
    bool draw_teammates = false;
    bool draw_head = true;
    bool draw_spectators = true;
    bool draw_radar = true;
    int chams_style = 3;

    bool use_vsync = true;
    float target_fps = 1000.0f;

    // ---- ESP Theme ----
    bool esp_use_theme = true;
    // Theme color is the "base" accent: box will be darkened, text/icons will be lightened
    float esp_theme_color[4] = {1.0f, 0.55f, 0.0f, 1.0f};  // default: orange

    float enemy_fill[4]    = {0.86f, 0.16f, 0.16f, 0.5f};
    float enemy_outline[4] = {1.00f, 0.24f, 0.24f, 0.35f};
    float enemy_glow[4]    = {1.00f, 0.20f, 0.20f, 0.10f};
    float team_fill[4]     = {0.16f, 0.39f, 0.86f, 0.5f};
    float team_outline[4]  = {0.24f, 0.51f, 1.00f, 0.35f};
    float team_glow[4]     = {0.20f, 0.39f, 1.00f, 0.10f};

    float body_width_scale = 1.0f;
    float head_radius = 6.0f;
    float depth_scale = 500.0f;
    float glow_expand_outer = 6.0f;
    float glow_expand_inner = 3.0f;

    bool menu_open = false;

    // ESP Opacity Drop (distance-based)
    bool  esp_opacity_drop         = true;
    float esp_opacity_drop_start   = 1500.0f;   // distance at which fade begins
    float esp_opacity_drop_end     = 3000.0f;  // distance at which fully faded
    float esp_opacity_drop_min     = 0.1f;     // minimum alpha multiplier at max distance

    // Radar defaults (tuned for 4K overlay alignment)
    float radar_size = BASE_RADAR_SIZE;
    float radar_zoom = 0.70f;
    float radar_x = BASE_RADAR_POS;
    float radar_y = BASE_RADAR_POS;
    bool radar_rotate = true;
    bool radar_always_centered = false; // match CS2 cl_radar_always_centered
    bool radar_circle = true;
    bool radar_auto_square = true;
    bool radar_rings = false;
    bool radar_names = false;
    bool radar_place_list = true;
    float radar_names_font_size = BASE_RADAR_NAMES_FONT_SIZE;
    float radar_list_font_size = BASE_RADAR_LIST_FONT_SIZE;
    float radar_bg_alpha = 0.10f;
    float radar_enemy_color[4] = {1.00f, 0.30f, 0.30f, 0.90f};
    float radar_team_color[4]  = {0.30f, 0.55f, 1.00f, 0.90f};

    int box_style = 0;
    float box_thickness = 1.5f;
    float box_padding_x = 5.0f;
    float box_padding_y = 5.0f;
    float box_corner_pct = 0.2f;

    // ESP Font — -1 means scan_fonts() will auto-select Tahoma (or first available)
    int esp_font_index = -1;
    float esp_font_atlas_size = 12.0f;

    // Name ESP
    int name_position = 0;
    float name_offset_x = 0.0f;
    float name_offset_y = -3.0f;
    float name_color[4] = {0.1f, 1.0f, 0.9f, 0.9f};
    float name_shadow_color[4] = {0.0f, 0.0f, 0.0f, 0.4f};
    bool name_shadow = true;
    float name_font_size = 12.0f;

    // Health bar
    bool healthbar_solid_color = false;       // if true: flat color instead of green->red gradient
    float healthbar_color[4] = {0.2f, 0.85f, 1.0f, 0.85f};  // solid color when enabled

    // Health text
    float hp_text_color[4] = {1.0f, 1.0f, 1.0f, 0.7f};
    float hp_text_shadow_color[4] = {0.0f, 0.0f, 0.0f, 0.4f};
    bool hp_text_shadow = true;
    float hp_font_size = 12.0f;

    // Weapon ESP
    float weapon_color[4] = {0.8f, 0.8f, 0.8f, 0.85f};
    float weapon_shadow_color[4] = {0.0f, 0.0f, 0.0f, 0.5f};
    float weapon_icon_color[4] = {1.0f, 1.0f, 1.0f, 0.9f};
    bool weapon_shadow = true;
    float weapon_font_size = 11.0f;
    bool weapon_show_icon = true;
    bool weapon_show_text = false;
    float weapon_distance_dropoff = 0.3f;

    // Spectator list position
    float spec_x = -1.0f;
    float spec_y = 30.0f;

    // Aimbot
    bool aimbot_enabled = false;
    int key_aimbot = 'X';
    int aimbot_fov = 5;
    float aimbot_smooth = 1.0f;
    bool aimbot_head_only = true;

    // Trigger
    bool  triggerbot_enabled = false;
    int   key_triggerbot     = 'X';
    int   triggerbot_delay   = 50;
    bool  triggerbot_head_only = true;

    // Crosshair
    bool crosshair_enabled = false;
    int crosshair_shape = 0;
    float crosshair_size = 5.0f;
    float crosshair_gap = 2.0f;
    float crosshair_thickness = 1.5f;
    float crosshair_color[4] = {0.0f, 1.0f, 0.0f, 1.0f};
    bool crosshair_outline = false;
    float crosshair_outline_thickness = 1.0f;
    float crosshair_outline_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    bool crosshair_dot = false;
    float crosshair_dot_size = 1.5f;

    // Menu
    float menu_x = -1.0f;
    float menu_y = -1.0f;
    int menu_tab = 0;

    // Menu style
    float menu_accent_color[4] = {1.0f, 0.55f, 0.0f, 1.0f};
    float menu_bg_alpha = 0.92f;
    float menu_border_color[4] = {1.0f, 0.55f, 0.0f, 0.3f};

    // Menu font
    int menu_font_index = -1;
    float menu_font_size = BASE_MENU_FONT_SIZE;

    // Key binds
    int key_menu = VK_NUMPAD1;
    int key_master = VK_NUMPAD2;
    int key_exit = VK_NUMPAD3;

    void reset() { *this = MenuSettings{}; }

    void apply_legacy_color_migration() {
        auto near_rgb = [](const float c[4], float r, float g, float b) {
            return std::fabs(c[0] - r) < 0.06f &&
                   std::fabs(c[1] - g) < 0.06f &&
                   std::fabs(c[2] - b) < 0.06f;
        };

        if (near_rgb(menu_accent_color, 0.0f, 1.0f, 0.65f)) {
            menu_accent_color[0] = 1.0f; menu_accent_color[1] = 0.55f;
            menu_accent_color[2] = 0.0f; menu_accent_color[3] = 1.0f;
        }
        if (near_rgb(menu_border_color, 0.0f, 1.0f, 0.65f)) {
            menu_border_color[0] = 1.0f; menu_border_color[1] = 0.55f;
            menu_border_color[2] = 0.0f; menu_border_color[3] = 0.3f;
        }
        if (near_rgb(esp_theme_color, 0.235f, 0.68f, 0.93f)) {
            esp_theme_color[0] = 1.0f; esp_theme_color[1] = 0.55f;
            esp_theme_color[2] = 0.0f; esp_theme_color[3] = 1.0f;
        }
    }

    // Only apply resolution-based defaults on first run (no saved config).
    void apply_startup_adapt(float screen_w, float screen_h) {
        apply_legacy_color_migration();
        if (g_config_loaded_from_disk)
            return;

        float s = resolution_scale(screen_w, screen_h);
        if (s > 1.01f)
            spec_y = BASE_SPEC_Y * s;
    }
};

inline MenuSettings g_settings;

// ============================================================
//  ESP Theme helpers
//  When esp_use_theme is true, all per-element colors are
//  derived from esp_theme_color instead of their individual
//  settings.  Enemy uses the raw hue; team uses a blue-shifted
//  complementary tint.
// ============================================================
namespace EspTheme {

    // Scale an rgb channel, clamped to [0,1]
    inline float sc(float v, float f) {
        return v * f < 0.0f ? 0.0f : (v * f > 1.0f ? 1.0f : v * f);
    }

    // Returns a derived color: hue from base rgb scaled by `bright`,
    // alpha = base[3] * a_ratio  (so the theme color picker's alpha is honoured).
    inline void derive(const float base[4], float bright, float a_ratio, float out[4]) {
        out[0] = sc(base[0], bright);
        out[1] = sc(base[1], bright);
        out[2] = sc(base[2], bright);
        out[3] = sc(base[3], a_ratio);   // <-- was: out[3] = a_ratio  (ignored base alpha)
    }

    // Blue-shift for teammates: blend hue toward blue, same alpha fix.
    inline void derive_team(const float base[4], float bright, float a_ratio, float out[4]) {
        float blue[4] = {0.2f, 0.45f, 1.0f, 1.0f};
        out[0] = sc(base[0] * 0.35f + blue[0] * 0.65f, bright);
        out[1] = sc(base[1] * 0.35f + blue[1] * 0.65f, bright);
        out[2] = sc(base[2] * 0.35f + blue[2] * 0.65f, bright);
        out[3] = sc(base[3], a_ratio);   // <-- same fix
    }

    // Fill all theme-driven color arrays in g_settings for preview/live use.
    // Called only when esp_use_theme == true.
    inline void apply(bool enemy) {
        const float* t = g_settings.esp_theme_color;

        if (enemy) {
            // Box outline: dark (0.5 brightness), medium alpha
            derive(t, 0.55f, 0.40f, g_settings.enemy_outline);
            // Chams fill: very dark, low alpha
            derive(t, 0.28f, 0.28f, g_settings.enemy_fill);
            // Glow: dark, very low alpha
            derive(t, 0.45f, 0.10f, g_settings.enemy_glow);
            // Name text: bright
            derive(t, 1.15f, 0.92f, g_settings.name_color);
            // Weapon text: slightly dimmer
            derive(t, 0.95f, 0.85f, g_settings.weapon_color);
            // Weapon icon: bright white-tinted
            derive(t, 1.10f, 0.90f, g_settings.weapon_icon_color);
            // Healthbar solid color: bright theme color
            derive(t, 1.05f, 0.85f, g_settings.healthbar_color);
            // HP text: bright theme hue, slightly dimmer than name
            derive(t, 1.05f, 0.86f, g_settings.hp_text_color);
        } else {
            derive_team(t, 0.55f, 0.40f, g_settings.team_outline);
            derive_team(t, 0.28f, 0.28f, g_settings.team_fill);
            derive_team(t, 0.45f, 0.10f, g_settings.team_glow);
        }
    }

} // namespace EspTheme

inline float esp_depth_opacity(float depth) {
    if (!g_settings.esp_opacity_drop) return 1.0f;
    float start = g_settings.esp_opacity_drop_start;
    float end   = g_settings.esp_opacity_drop_end;
    float mn    = g_settings.esp_opacity_drop_min;
    if (depth <= start) return 1.0f;
    if (depth >= end)   return mn;
    float t = (depth - start) / (end - start);
    return 1.0f - t * (1.0f - mn);
}