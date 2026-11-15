#pragma once
#include <imgui.h>
#include <vector>
#include <string>
#include <cstdint>
#include <mutex>
#include "memory/memory_syscall.h"
#include "offsets.h"
#include "settings.h"
#include "entity_utils.h"
#include "overlay.h"

class SpectatorList {
public:
    std::vector<std::string> spectators;
    mutable std::mutex mtx;

    void update(uintptr_t entity_list, uintptr_t local_pawn, uintptr_t local_controller) {
        std::vector<std::string> new_list;
        new_list.reserve(8);
        if (!entity_list || !local_pawn) return;

        // Local handles we may be observed as (player pawn / controller pawn / index).
        uint32_t local_h_player = 0;
        uint32_t local_h_pawn = 0;
        uint32_t local_h_obs = 0;
        if (local_controller) {
            local_h_player = g_memory->read<uint32_t>(
                local_controller + g_offsets.CCSPlayerController.m_hPlayerPawn);
            local_h_pawn = g_memory->read<uint32_t>(
                local_controller + g_offsets.CCSPlayerController.m_hPawn);
            if (g_offsets.CCSPlayerController.m_hObserverPawn)
                local_h_obs = g_memory->read<uint32_t>(
                    local_controller + g_offsets.CCSPlayerController.m_hObserverPawn);
        }

        auto handle_index = [](uint32_t h) -> uint32_t {
            return h & EntityList::HANDLE_MASK;
        };

        auto is_local_target = [&](uint32_t obs_target) -> bool {
            if (!obs_target || obs_target == 0xFFFFFFFF) return false;

            if (pawn_handle_ok(local_h_player) && obs_target == local_h_player) return true;
            if (pawn_handle_ok(local_h_pawn) && obs_target == local_h_pawn) return true;
            if (pawn_handle_ok(local_h_obs) && obs_target == local_h_obs) return true;

            // Serial can differ; match by entity index.
            const uint32_t ti = handle_index(obs_target);
            if (pawn_handle_ok(local_h_player) && ti == handle_index(local_h_player)) return true;
            if (pawn_handle_ok(local_h_pawn) && ti == handle_index(local_h_pawn)) return true;
            if (pawn_handle_ok(local_h_obs) && ti == handle_index(local_h_obs)) return true;

            uintptr_t resolved = EntityList::resolve_handle(entity_list, obs_target);
            if (resolved && resolved == local_pawn) return true;
            return false;
        };

        auto read_obs_target = [&](uintptr_t pawn) -> uint32_t {
            if (!pawn) return 0;
            uintptr_t obs_svc = g_memory->read<uintptr_t>(
                pawn + g_offsets.C_BasePlayerPawn.m_pObserverServices);
            if (!obs_svc) return 0;
            return g_memory->read<uint32_t>(
                obs_svc + g_offsets.CPlayer_ObserverServices.m_hObserverTarget);
        };

        uintptr_t first_page = g_memory->read<uintptr_t>(
            entity_list + EntityList::PAGE_HEADER);
        if (!first_page) return;

        for (int i = 1; i < EntityList::MAX_PLAYERS; i++) {
            uintptr_t controller = g_memory->read<uintptr_t>(
                first_page + EntityList::ENTRY_STRIDE * (i & EntityList::INDEX_MASK));
            if (!controller || controller == local_controller) continue;

            char name[128]{};
            read_player_name(controller, name, sizeof(name));
            if (!name[0]) continue;

            // Prefer observer pawn for services (dead / pure spectators).
            uint32_t obs_pawn_handle = 0;
            if (g_offsets.CCSPlayerController.m_hObserverPawn)
                obs_pawn_handle = g_memory->read<uint32_t>(
                    controller + g_offsets.CCSPlayerController.m_hObserverPawn);

            uint32_t player_pawn_handle = get_pawn_handle(controller);

            uintptr_t obs_pawn = pawn_handle_ok(obs_pawn_handle)
                ? EntityList::resolve_handle(entity_list, obs_pawn_handle) : 0;
            uintptr_t player_pawn = pawn_handle_ok(player_pawn_handle)
                ? EntityList::resolve_handle(entity_list, player_pawn_handle) : 0;

            if ((!obs_pawn || obs_pawn == local_pawn) &&
                (!player_pawn || player_pawn == local_pawn))
                continue;

            // Skip living combatants (team 1 = pure spectator).
            if (player_pawn) {
                int health = g_memory->read<int>(player_pawn + g_offsets.C_BaseEntity.m_iHealth);
                int team = g_memory->read<int>(player_pawn + g_offsets.C_BaseEntity.m_iTeamNum);
                if (health > 0 && team != 1)
                    continue;
            }

            uint32_t obs_target = read_obs_target(obs_pawn);
            if (!is_local_target(obs_target))
                obs_target = read_obs_target(player_pawn);
            if (!is_local_target(obs_target))
                continue;

            bool dup = false;
            for (const auto& s : new_list)
                if (s == name) { dup = true; break; }
            if (!dup) new_list.push_back(name);
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            spectators = std::move(new_list);
        }
    }

    void draw(int screen_w) {
        if (!g_settings.draw_spectators || !g_settings.master_switch) return;

        char title[64];
        snprintf(title, sizeof(title), "观战者 (%d)###spec", (int)spectators.size());

        float window_w = 280.0f;

        float sx = g_settings.spec_x;
        float sy = g_settings.spec_y;
        if (sx < 0) sx = (float)screen_w - window_w - 10.0f;

        ImGui::SetNextWindowPos({sx, sy}, ImGuiCond_Always);
        ImGui::SetNextWindowSizeConstraints({window_w, 0}, {window_w, 1000});
        ImGui::SetNextWindowBgAlpha(0.7f);

        int flags = ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_NoFocusOnAppearing |
                    ImGuiWindowFlags_NoNav |
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoInputs;

        ImFont* font = g_overlay.spec_font;
        if (font) ImGui::PushFont(font);

        ImGui::Begin(title, nullptr, flags);

        {
            std::lock_guard<std::mutex> lock(mtx);
            for (const auto& s : spectators)
                ImGui::TextColored({1.0f, 0.4f, 0.4f, 1}, "> %s", s.c_str());
        }

        ImGui::End();

        if (font) ImGui::PopFont();
    }
};

inline SpectatorList g_spectators;
