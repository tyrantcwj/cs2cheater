#pragma once
#include <cmath>
#include <cstring>
#include <cstdio>
#include "types.h"
#include "memory/memory_syscall.h"
#include "offsets.h"
#include "entity_utils.h"
#include "map_place.h"

struct MapOverview {
    float pos_x = 0.0f;
    float pos_y = 0.0f;
    float scale = 5.0f;
    float inset_left = 0.0f;
    float inset_top = 0.0f;
    float inset_right = 0.0f;
    float inset_bottom = 0.0f;
    bool valid = false;
};

inline MapOverview get_map_overview(const std::string& map_name) {
    if (map_name.find("ar_baggage") != std::string::npos)       return {-1316.0f, 1288.0f, 2.539062f, 0.0f, 0.0f, 0.0f, 0.0f, true};
    if (map_name.find("ar_shoots") != std::string::npos)        return {-1368.0f, 1952.0f, 2.687500f, 0.0f, 0.0f, 0.0f, 0.0f, true};
    if (map_name.find("cs_italy") != std::string::npos)         return {-2647.0f, 2592.0f, 4.600000f, 0.0f, 0.0f, 0.0f, 0.0f, true};
    if (map_name.find("cs_office") != std::string::npos)        return {-1838.0f, 1858.0f, 4.100000f, 0.0f, 0.0f, 0.0f, 0.0f, true};

    // Check ancient before just "de_ancient" to catch v1/v2/night automatically
    if (map_name.find("de_ancient") != std::string::npos)       return {-2953.0f, 2164.0f, 5.000000f, 0.0f, 0.0f, 0.0f, 0.0f, true};
    if (map_name.find("de_anubis") != std::string::npos)        return {-2796.0f, 3328.0f, 5.220000f, 0.0f, 0.0f, 0.0f, 0.0f, true};
    if (map_name.find("de_cache") != std::string::npos)         return {-2000.0f, 3250.0f, 5.500000f, 0.16f, 0.04f, 0.20f, 0.04f, true};
    if (map_name.find("de_cbble") != std::string::npos)         return {-3615.0f, 2712.0f, 4.724792f, 0.15f, 0.01f, 0.30f, 0.15f, true};

    // Check dust2 BEFORE dust
    if (map_name.find("de_dust2") != std::string::npos)         return {-2476.0f, 3239.0f, 4.400000f, 0.0f, 0.0f, 0.0f, 0.0f, true};
    if (map_name.find("de_dust") != std::string::npos)          return {-2850.0f, 4073.0f, 6.000000f, 0.0f, 0.0f, 0.0f, 0.0f, true};

    if (map_name.find("de_inferno") != std::string::npos)       return {-2087.0f, 3870.0f, 4.900000f, 0.0f, 0.0f, 0.0f, 0.0f, true};
    if (map_name.find("de_mirage") != std::string::npos)        return {-3230.0f, 1713.0f, 5.000000f, 0.135f, 0.08f, 0.105f, 0.08f, true};
    if (map_name.find("de_nuke") != std::string::npos)          return {-3453.0f, 2887.0f, 7.000000f, 0.33f, 0.20f, 0.20f, 0.20f, true};
    if (map_name.find("de_overpass") != std::string::npos)      return {-4831.0f, 1781.0f, 5.200000f, 0.0f, 0.0f, 0.0f, 0.0f, true};
    if (map_name.find("de_train") != std::string::npos)         return {-2308.0f, 2078.0f, 4.082077f, 0.0f, 0.0f, 0.0f, 0.0f, true};
    if (map_name.find("de_vertigo") != std::string::npos)       return {-3168.0f, 1762.0f, 4.000000f, 0.10f, 0.10f, 0.20f, 0.15f, true};
    if (map_name.find("workshop_preview") != std::string::npos) return {-2071.0f,  711.0f, 1.699219f, 0.0f, 0.0f, 0.0f, 0.0f, true};

    return {}; // Default fallback if unknown map
}

inline float get_map_scale(const std::string& map_name) {
    return get_map_overview(map_name).scale;
}

struct WeaponInfo {
    uint16_t def_index;
    const char* name;
};

static constexpr WeaponInfo WEAPON_TABLE[] = {
    {1,   "Desert Eagle"},
    {2,   "Dual Berettas"},
    {3,   "Five-SeveN"},
    {4,   "Glock-18"},
    {7,   "AK-47"},
    {8,   "AUG"},
    {9,   "AWP"},
    {10,  "FAMAS"},
    {11,  "G3SG1"},
    {13,  "Galil AR"},
    {14,  "M249"},
    {16,  "M4A4"},
    {17,  "MAC-10"},
    {19,  "P90"},
    {23,  "MP5-SD"},
    {24,  "UMP-45"},
    {25,  "XM1014"},
    {26,  "PP-Bizon"},
    {27,  "MAG-7"},
    {28,  "Negev"},
    {29,  "Sawed-Off"},
    {30,  "Tec-9"},
    {31,  "Zeus x27"},
    {32,  "P2000"},
    {33,  "MP7"},
    {34,  "MP9"},
    {35,  "Nova"},
    {36,  "P250"},
    {38,  "SCAR-20"},
    {39,  "SG 553"},
    {40,  "SSG 08"},
    {42,  "Knife"},
    {43,  "Flashbang"},
    {44,  "HE Grenade"},
    {45,  "Smoke"},
    {46,  "Molotov"},
    {47,  "Decoy"},
    {48,  "Incendiary"},
    {49,  "C4"},
    {59,  "Knife"},
    {60,  "M4A1-S"},
    {61,  "USP-S"},
    {63,  "CZ75-Auto"},
    {64,  "R8 Revolver"},
    {500, "Bayonet"},
    {503, "Shadow Daggers"},
    {505, "Flip Knife"},
    {506, "Gut Knife"},
    {507, "Karambit"},
    {508, "M9 Bayonet"},
    {509, "Huntsman Knife"},
    {512, "Falchion Knife"},
    {514, "Bowie Knife"},
    {515, "Butterfly Knife"},
    {516, "Ursus Knife"},
    {517, "Navaja Knife"},
    {518, "Stiletto Knife"},
    {519, "Talon Knife"},
    {520, "Skeleton Knife"},
    {521, "Nomad Knife"},
    {522, "Survival Knife"},
    {523, "Paracord Knife"},
    {525, "Classic Knife"},
    {526, "Kukri Knife"},
};
static constexpr int WEAPON_TABLE_SIZE = sizeof(WEAPON_TABLE) / sizeof(WEAPON_TABLE[0]);

inline const WeaponInfo* lookup_weapon(uint16_t def_index) {
    for (int i = 0; i < WEAPON_TABLE_SIZE; i++)
        if (WEAPON_TABLE[i].def_index == def_index)
            return &WEAPON_TABLE[i];
    return nullptr;
}

struct CameraState {
    Vec3  origin{};       // exact eye position
    Vec3  angles{};       // exact view angles (pitch, yaw, roll)
    float fov = 90.0f;    // exact rendered FOV
    bool  valid = false;
};

struct LocalPlayerState {
    uintptr_t pawn = 0;
    uintptr_t observer_pawn = 0;
    uintptr_t controller = 0;
    int team = 0;
    int health = 0;
    bool alive = false;
    float x = 0, y = 0, z = 0, yaw = 0;
    bool is_scoped = false;
    CameraState camera;
};

struct FrameState {
    Matrix4x4 view_matrix{};
    LocalPlayerState local;
    PlayerVisuals players[64];
    RadarPlayer radar_players[64];
    uintptr_t entity_list = 0;
    std::string map_name;
    float map_scale = 5.0f;
    MapOverview map_overview{};
};

// -----------------------------------------------------------------------
// Compact pawn data we can read in a single bulk RPM call.
// Covers all offsets we need from a pawn pointer.
// We read a window of bytes and index into it locally — zero extra RPM calls
// once we have this buffer.
// -----------------------------------------------------------------------
struct PawnSnapshot {
    // Raw byte buffer covering offset 0 up to the highest offset we need.
    // Highest needed: max of (m_iHealth, m_iTeamNum, m_pGameSceneNode,
    //                         m_bIsScoped, m_pObserverServices, m_pClippingWeapon)
    // These are typically all within the first 0x800 bytes of the pawn.
    static constexpr size_t SIZE = 0x800;
    uint8_t buf[SIZE];

    template<typename T>
    T get(uint32_t offset) const {
        if (offset + sizeof(T) > SIZE) return T{};
        T val;
        memcpy(&val, buf + offset, sizeof(T));
        return val;
    }
};

class EntityReader {
public:
    FrameState read_frame(int screen_w, int screen_h) {
        FrameState state{};

        // --- NEW: Read GlobalVars for Map Scale (Cached) ---
        uintptr_t global_vars = g_memory->read<uintptr_t>(
            g_memory->get_client_base() + g_offsets.client.dwGlobalVars);

        if (global_vars) {
            // 0x188 is the standard offset for m_currentMapName in CGlobalVarsBase
            uintptr_t current_map_ptr = g_memory->read<uintptr_t>(global_vars + 0x188);

            // If the pointer changed (e.g., map changed or joined new server), read the new string
            if (current_map_ptr != cached_map_ptr && current_map_ptr != 0) {
                char map_name[64] = {0};
                if (g_memory->read_raw(current_map_ptr, map_name, sizeof(map_name))) {
                    cached_map_overview = get_map_overview(map_name);
                    cached_map_scale = cached_map_overview.scale;
                    cached_map_ptr = current_map_ptr;
                    cached_map_name = map_name;
                }
            }
        }
        state.map_scale = cached_map_scale; // Assign the cached scale to the frame state
        state.map_overview = cached_map_overview;
        state.map_name = cached_map_name;
        // ---------------------------------------------------

        // --- 1 RPM: view matrix ---
        g_memory->read_raw(g_memory->get_client_base() + g_offsets.client.dwViewMatrix,
                          &state.view_matrix, 64);

        // --- 1 RPM: entity list ptr (needed before handle resolution) ---
        int highest_entity = -1;
        state.entity_list = resolve_entity_system(&highest_entity);
        if (!state.entity_list) return state;

        // --- 1 RPM: local pawn ptr ---
        state.local.pawn = g_memory->read<uintptr_t>(
            g_memory->get_client_base() + g_offsets.client.dwLocalPlayerPawn);
        // --- 1 RPM: local controller ptr ---
        state.local.controller = g_memory->read<uintptr_t>(
            g_memory->get_client_base() + g_offsets.client.dwLocalPlayerController);

        state.local.observer_pawn = 0;
        if (state.local.controller && g_offsets.CCSPlayerController.m_hObserverPawn) {
            uint32_t obs_h = g_memory->read<uint32_t>(
                state.local.controller + g_offsets.CCSPlayerController.m_hObserverPawn);
            if (pawn_handle_ok(obs_h))
                state.local.observer_pawn = EntityList::resolve_handle(state.entity_list, obs_h);
        }

        if (state.local.pawn) {
            // --- 1 RPM: bulk-read local pawn snapshot ---
            PawnSnapshot local_snap{};
            g_memory->read_raw(state.local.pawn, local_snap.buf, PawnSnapshot::SIZE);

            state.local.team = local_snap.get<int>(g_offsets.C_BaseEntity.m_iTeamNum);
            state.local.health = local_snap.get<int>(g_offsets.C_BaseEntity.m_iHealth);
            state.local.alive = (state.local.health > 0);
            state.local.is_scoped = local_snap.get<bool>(g_offsets.C_CSPlayerPawn.m_bIsScoped);

            uintptr_t local_scene = local_snap.get<uintptr_t>(
                g_offsets.C_BaseEntity.m_pGameSceneNode);
            if (local_scene) {
                // --- 1 RPM: local scene origin ---
                Vec3 origin = g_memory->read<Vec3>(
                    local_scene + g_offsets.CGameSceneNode.m_vecAbsOrigin);
                state.local.x = origin.x;
                state.local.y = origin.y;
                state.local.z = origin.z;
            }
            state.local.yaw = atan2f(state.view_matrix.m[0][1],
                                     state.view_matrix.m[0][0])
                              * 180.0f / 3.14159265f - 90.0f;
        } else if (state.local.controller) {
            state.local.team = g_memory->read<int>(
                state.local.controller + g_offsets.C_BaseEntity.m_iTeamNum);
        }

        // When dead and spectating a teammate, measure place-list distance from the spectated player.
        if (!state.local.alive && state.local.observer_pawn) {
            uintptr_t obs_svc = g_memory->read<uintptr_t>(
                state.local.observer_pawn + g_offsets.C_BasePlayerPawn.m_pObserverServices);
            if (obs_svc) {
                uint32_t obs_target_h = g_memory->read<uint32_t>(
                    obs_svc + g_offsets.CPlayer_ObserverServices.m_hObserverTarget);
                uintptr_t obs_target = pawn_handle_ok(obs_target_h)
                    ? EntityList::resolve_handle(state.entity_list, obs_target_h) : 0;
                if (obs_target) {
                    uintptr_t obs_scene = g_memory->read<uintptr_t>(
                        obs_target + g_offsets.C_BaseEntity.m_pGameSceneNode);
                    if (obs_scene) {
                        Vec3 obs_origin = g_memory->read<Vec3>(
                            obs_scene + g_offsets.CGameSceneNode.m_vecAbsOrigin);
                        state.local.x = obs_origin.x;
                        state.local.y = obs_origin.y;
                        state.local.z = obs_origin.z;
                    }
                }
            }
        }

        state.local.camera = read_camera();

        // --- 1 RPM: first page ptr ---
        uintptr_t first_page = g_memory->read<uintptr_t>(
            state.entity_list + EntityList::PAGE_HEADER);
        if (!first_page) return state;

        // ---------------------------------------------------------------
        // KEY OPTIMIZATION: read the entire entity page in ONE RPM call.
        // Previously: 63 individual read<uintptr_t> calls = 63 syscalls.
        // Now: 1 bulk read of 7056 bytes = 1 syscall.
        // Each slot is 112 bytes (ENTRY_STRIDE); we need the pointer at
        // offset 0 within each slot.
        // ---------------------------------------------------------------
        static constexpr size_t PAGE_BUF_SIZE =
            EntityList::ENTRY_STRIDE * 512;
        static uint8_t page_buf[PAGE_BUF_SIZE];

        if (!g_memory->read_raw(first_page, page_buf, PAGE_BUF_SIZE)) {
            for (int slot = 0; slot < EntityList::MAX_PLAYERS; slot++) {
                uintptr_t controller = g_memory->read<uintptr_t>(
                    first_page + EntityList::ENTRY_STRIDE * (slot & EntityList::INDEX_MASK));
                memcpy(page_buf + EntityList::ENTRY_STRIDE * (slot & EntityList::INDEX_MASK),
                       &controller, sizeof(controller));
            }
        }

        for (int i = 1; i < EntityList::MAX_PLAYERS; i++) {
            uintptr_t controller;
            memcpy(&controller,
                   page_buf + EntityList::ENTRY_STRIDE * (i & EntityList::INDEX_MASK),
                   sizeof(uintptr_t));
            if (!controller) continue;

            read_player(state, controller, i, screen_w, screen_h);
        }

        return state;
    }

private:
    CBoneData bone_buf[MAX_BONE];
    uintptr_t cached_map_ptr = 0;
    float cached_map_scale = 5.0f;
    MapOverview cached_map_overview{};
    std::string cached_map_name;
    std::chrono::steady_clock::time_point last_spotted_time[64];

    CameraState read_camera()
    {
        CameraState cam;

        uintptr_t view_render = g_memory->read<uintptr_t>(
            g_memory->get_client_base() + g_offsets.client.dwViewRender);

        if (!view_render)
            return cam;

        uintptr_t view = view_render + 0x10;

        struct ViewData {
            Vec3  origin;   // +0x00
            Vec3  angles;   // +0x0C
            float fov;      // +0x18
        };

        ViewData data{};
        if (!g_memory->read_raw(view, &data, sizeof(data)))
            return cam;

        if (data.fov <= 0.0f || data.fov > 170.0f)
            data.fov = 90.0f;

        cam.origin = data.origin;
        cam.angles = data.angles;
        cam.fov    = data.fov;
        cam.valid  = true;

        return cam;
    }

    void read_weapon(uintptr_t pawn, uintptr_t entity_list,
                     char* out_name, size_t max_len, uint16_t& out_def_index) {
        out_name[0] = 0;
        out_def_index = 0;

        // 1. Weapon services
        uintptr_t weapon_services = g_memory->read<uintptr_t>(
            pawn + g_offsets.C_CSPlayerPawnBase.m_pWeaponServices);
        if (!weapon_services) return;

        // 2. Active weapon handle
        uint32_t handle = g_memory->read<uint32_t>(
            weapon_services + g_offsets.CPlayer_WeaponServices.m_hActiveWeapon);
        if ((handle & 0xFFFFFF) == 0xFFFFFF) return;

        // 3. Resolve via your existing helper
        uintptr_t weapon = EntityList::resolve_handle(entity_list, handle);
        if (!weapon) return;

        // 4. Def index — same as before
        uint16_t def_index = g_memory->read<uint16_t>(
            weapon + g_offsets.C_EconEntity.m_AttributeManager
                   + g_offsets.C_AttributeContainer.m_Item
                   + g_offsets.C_EconItemView.m_iItemDefinitionIndex);
        if (def_index == 0) return;

        out_def_index = def_index;
        const WeaponInfo* info = lookup_weapon(def_index);
        if (info)
            snprintf(out_name, max_len, "%s", info->name);
        else
            snprintf(out_name, max_len, "Weapon %d", def_index);
    }

    void read_player(FrameState& state, uintptr_t controller, int i,
                     int screen_w, int screen_h) {
        char name[128]{};
        read_player_name(controller, name, sizeof(name));

        uint32_t pawn_handle = get_pawn_handle(controller);
        if (!pawn_handle) return;

        uintptr_t pawn = EntityList::resolve_handle(state.entity_list, pawn_handle);
        if (!pawn || pawn == state.local.pawn) return;

        // ---------------------------------------------------------------
        // KEY OPTIMIZATION: bulk-read pawn in one RPM call.
        // Previously: separate RPM calls for health, team, scene_node, origin.
        // Now: 1 RPM call, then local memcpy for each field.
        // ---------------------------------------------------------------
        PawnSnapshot snap{};
        if (!g_memory->read_raw(pawn, snap.buf, PawnSnapshot::SIZE)) return;

        int health = snap.get<int>(g_offsets.C_BaseEntity.m_iHealth);
        if (health <= 0) return;

        int team = snap.get<int>(g_offsets.C_BaseEntity.m_iTeamNum);

        uintptr_t scene_node = snap.get<uintptr_t>(g_offsets.C_BaseEntity.m_pGameSceneNode);
        if (!scene_node) return;

        // --- 1 RPM: scene origin (external ptr, can't batch with pawn) ---
        Vec3 origin = g_memory->read<Vec3>(
            scene_node + g_offsets.CGameSceneNode.m_vecAbsOrigin);

        // --- 1 RPM: bone array ptr (inside scene_node) ---
        uintptr_t bone_array = g_memory->read<uintptr_t>(
            scene_node + g_offsets.CSkeletonInstance.m_modelState + 0x80);
        const bool have_bones = bone_array &&
            g_memory->read_raw(bone_array, bone_buf, sizeof(bone_buf));

        auto& player = state.players[i];
        player.valid = true;
        player.team = team;
        player.health = health;
        player.origin = origin;
        if (have_bones) {
            player.head_world   = bone_buf[BONE_HEAD].pos;
            player.neck_world   = bone_buf[BONE_NECK].pos;
            player.chest_world  = bone_buf[BONE_CHEST].pos;
            player.pelvis_world = bone_buf[BONE_PELVIS].pos;
        } else {
            player.head_world   = { origin.x, origin.y, origin.z + 72.0f };
            player.neck_world   = { origin.x, origin.y, origin.z + 64.0f };
            player.chest_world  = { origin.x, origin.y, origin.z + 48.0f };
            player.pelvis_world = { origin.x, origin.y, origin.z + 36.0f };
        }

        memcpy(player.name, name, 128);

        char place_raw[18]{};
        if (g_offsets.C_CSPlayerPawn.m_szLastPlaceName)
            g_memory->read_raw(pawn + g_offsets.C_CSPlayerPawn.m_szLastPlaceName,
                               place_raw, sizeof(place_raw));
        place_raw[sizeof(place_raw) - 1] = 0;
        for (size_t n = 0; place_raw[n]; n++)
            if ((unsigned char)place_raw[n] < 0x20) place_raw[n] = 0;

        const float dx = origin.x - state.local.x;
        const float dy = origin.y - state.local.y;
        const float dz = origin.z - state.local.z;
        const int meters = (int)(sqrtf(dx * dx + dy * dy + dz * dz) / 39.37f + 0.5f);

        const char* zh = resolve_place_name(place_raw, state.map_name.c_str(),
                                            origin.x, origin.y);
        if (zh && zh[0])
            snprintf(player.place, sizeof(player.place), "%s  %dm", zh, meters);
        else
            snprintf(player.place, sizeof(player.place), "%dm", meters);

        read_weapon(pawn, state.entity_list, player.weapon, sizeof(player.weapon), player.weapon_def_index);

        if (have_bones) {
            for (int b = 0; b < MAX_BONE; b++)
                player.visible[b] = w2s_depth(
                    bone_buf[b].pos, state.view_matrix,
                    screen_w, screen_h,
                    player.screens[b], player.depths[b]);
        } else {
            const Vec3 stand[] = {
                { origin.x, origin.y, origin.z + 72.0f },
                { origin.x, origin.y, origin.z + 36.0f },
                { origin.x, origin.y, origin.z },
            };
            const int ids[] = { BONE_HEAD, BONE_PELVIS, BONE_LFOOT };
            for (int n = 0; n < 3; n++)
                player.visible[ids[n]] = w2s_depth(
                    stand[n], state.view_matrix, screen_w, screen_h,
                    player.screens[ids[n]], player.depths[ids[n]]);
            player.visible[BONE_RFOOT] = player.visible[BONE_LFOOT];
            player.screens[BONE_RFOOT] = player.screens[BONE_LFOOT];
            player.depths[BONE_RFOOT]  = player.depths[BONE_LFOOT];
        }


        const bool is_spotted_ingame = g_memory->read<bool>(pawn
            + g_offsets.C_CSPlayerPawn.m_entitySpottedState
            + g_offsets.EntitySpottedState_t.m_bSpotted);

        auto now = std::chrono::steady_clock::now();
        if (is_spotted_ingame) {
            last_spotted_time[i] = now;
        }

        // Calculate how much time has passed since they were last spotted
        auto time_since_spotted = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_spotted_time[i]).count();

        bool is_recently_spotted = (time_since_spotted < 1000);

        auto& rp = state.radar_players[i];
        rp.x = origin.x;
        rp.y = origin.y;
        rp.z = origin.z;
        rp.team = team;
        rp.health = health;
        rp.valid = true;
        rp.is_spotted = is_recently_spotted;
        rp.meters = meters;
        memcpy(rp.name, name, 128);
        rp.place[0] = 0;
        if (zh && zh[0]) {
            strncpy(rp.place, zh, sizeof(rp.place) - 1);
            rp.place[sizeof(rp.place) - 1] = 0;
        }
    }
};