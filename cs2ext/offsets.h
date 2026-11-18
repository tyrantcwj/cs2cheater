#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <fstream>
#include <string>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "memory/imemory.h"

struct Offsets {
    struct {
        uint32_t dwEntityList, dwViewMatrix, dwViewRender, dwLocalPlayerPawn, dwLocalPlayerController, dwGlobalVars;
        uint32_t highestEntityIndex;
    } client;
    struct {
        uint32_t m_iTeamNum, m_pGameSceneNode, m_iHealth;
    } C_BaseEntity;
    struct {
        uint32_t m_vecAbsOrigin;
    } CGameSceneNode;
    struct {
        uint32_t m_modelState;
    } CSkeletonInstance;
    struct {
        uint32_t m_hPlayerPawn;
        uint32_t m_hPawn;
        uint32_t m_sSanitizedPlayerName;
        uint32_t m_iszPlayerName;
        uint32_t m_hObserverPawn;
    } CCSPlayerController;
    struct {
        uint32_t m_pObserverServices;
    } C_BasePlayerPawn;
    struct {
        uint32_t m_hObserverTarget;
    } CPlayer_ObserverServices;
    struct {
        uint32_t m_bIsScoped, m_entitySpottedState, m_szLastPlaceName;
    } C_CSPlayerPawn;
    struct {
        uint32_t m_vecViewOffset;
    } C_BaseModelEntity;
    struct {
        uint32_t m_pWeaponServices;
    } C_CSPlayerPawnBase;
    struct {
        uint32_t m_hActiveWeapon;
    } CPlayer_WeaponServices;
    struct {
        uint32_t m_AttributeManager;
    } C_EconEntity;
    struct {
        uint32_t m_Item;
    } C_AttributeContainer;
    struct {
        uint32_t m_iItemDefinitionIndex;
    } C_EconItemView;
    struct {
        uint32_t m_bSpotted;
    } EntitySpottedState_t;

    bool load(const std::string& offsets_path, const std::string& client_dll_path) {
        // Dump against the already-attached game every launch so a patch
        // does not wait for a failed read. A missing dumper still uses files.
        bool dumped = false;
        if (std::filesystem::exists("cs2-dumper.exe")) {
            printf("[*] 正在根据当前游戏自动更新偏移...\n");
            dumped = run_dumper();
            if (!dumped)
                printf("[!] 自动导出失败，尝试使用已有偏移文件\n");
        }

        if (!std::filesystem::exists(offsets_path) ||
            !std::filesystem::exists(client_dll_path)) {
            if (!dumped) {
                printf("[!] Offset files not found, running cs2-dumper...\n");
                if (!run_dumper()) return false;
            }
        }

        if (!parse_offsets(offsets_path, client_dll_path)) {
            printf("[!] Failed to parse offsets. Running cs2-dumper...\n");
            if (!run_dumper() || !parse_offsets(offsets_path, client_dll_path)) {
                printf("[!] Still failed after re-dump.\n");
                return false;
            }
        }

        TestResult result = TestResult::NOT_READY;
        for (int attempt = 0; attempt < 10; ++attempt) {
            result = functional_test();
            if (result != TestResult::NOT_READY)
                break;
            if (attempt == 0)
                printf("[*] 实体系统还没就绪，正在重试...\n");
            Sleep(200);
        }

        if (result == TestResult::OFFSETS_WRONG && !dumped) {
            printf("[!] Offsets are outdated (can't read valid game data). Running cs2-dumper...\n");
            if (!run_dumper() || !parse_offsets(offsets_path, client_dll_path)) {
                printf("[!] 导出偏移失败，继续使用当前偏移\n");
            } else {
                result = functional_test();
            }
        }

        if (result == TestResult::OFFSETS_WRONG) {
            printf("[!] Offsets still invalid after dump. Game may have updated.\n");
            printf("[*] 继续使用当前偏移\n");
        } else if (result == TestResult::NO_PLAYERS || result == TestResult::NOT_READY) {
            printf("[*] Offsets parsed OK but no players found (you may not be in a match)\n");
            printf("[*] Proceeding — offsets will be validated when players are present\n");
        } else if (result == TestResult::OK) {
            printf("[+] Offsets validated: successfully read player data\n");
        }

        return true;
    }

private:
    enum class TestResult { OK, NO_PLAYERS, NOT_READY, OFFSETS_WRONG };

    bool parse_offsets(const std::string& offsets_path, const std::string& client_dll_path) {
        try {
            nlohmann::json oj, cj;
            {
                std::ifstream f(offsets_path);
                if (!f) return false;
                f >> oj;
            }
            {
                std::ifstream f(client_dll_path);
                if (!f) return false;
                f >> cj;
            }

            auto& cl = oj["client.dll"];
            client.dwEntityList = cl["dwEntityList"];
            client.dwViewMatrix = cl["dwViewMatrix"];
            client.dwViewRender = cl["dwViewRender"];
            client.dwLocalPlayerPawn = cl["dwLocalPlayerPawn"];
            client.dwLocalPlayerController = cl["dwLocalPlayerController"];
            client.dwGlobalVars = cl["dwGlobalVars"];
            client.highestEntityIndex = cl.value("dwGameEntitySystem_highestEntityIndex", 0x2120);

            auto& cs = cj["client.dll"]["classes"];
            C_BaseEntity.m_iTeamNum = cs["C_BaseEntity"]["fields"]["m_iTeamNum"];
            C_BaseEntity.m_pGameSceneNode = cs["C_BaseEntity"]["fields"]["m_pGameSceneNode"];
            C_BaseEntity.m_iHealth = cs["C_BaseEntity"]["fields"]["m_iHealth"];
            CGameSceneNode.m_vecAbsOrigin = cs["CGameSceneNode"]["fields"]["m_vecAbsOrigin"];
            CSkeletonInstance.m_modelState = cs["CSkeletonInstance"]["fields"]["m_modelState"];

            CCSPlayerController.m_hPlayerPawn =
                cs["CCSPlayerController"]["fields"]["m_hPlayerPawn"];
            CCSPlayerController.m_sSanitizedPlayerName =
                cs["CCSPlayerController"]["fields"]["m_sSanitizedPlayerName"];
            CCSPlayerController.m_iszPlayerName =
                cs["CBasePlayerController"]["fields"].value("m_iszPlayerName", 0x6F4);
            CCSPlayerController.m_hPawn =
                cs["CBasePlayerController"]["fields"]["m_hPawn"];
            CCSPlayerController.m_hObserverPawn =
                cs["CCSPlayerController"]["fields"].value("m_hObserverPawn", 0x930);

            C_BasePlayerPawn.m_pObserverServices =
                cs["C_BasePlayerPawn"]["fields"]["m_pObserverServices"];
            CPlayer_ObserverServices.m_hObserverTarget =
                cs["CPlayer_ObserverServices"]["fields"]["m_hObserverTarget"];

            C_CSPlayerPawn.m_bIsScoped = cs["C_CSPlayerPawn"]["fields"]["m_bIsScoped"];
            C_CSPlayerPawn.m_szLastPlaceName =
                cs["C_CSPlayerPawn"]["fields"].value("m_szLastPlaceName", 0x14DC);
            C_BaseModelEntity.m_vecViewOffset =
                cs["C_BaseModelEntity"]["fields"]["m_vecViewOffset"];

            // Weapon reading offsets
            C_CSPlayerPawnBase.m_pWeaponServices = cs["C_BasePlayerPawn"]["fields"]["m_pWeaponServices"];
            CPlayer_WeaponServices.m_hActiveWeapon = cs["CPlayer_WeaponServices"]["fields"]["m_hActiveWeapon"];

            C_EconEntity.m_AttributeManager = cs["C_EconEntity"]["fields"]["m_AttributeManager"];
            C_AttributeContainer.m_Item = cs["C_AttributeContainer"]["fields"]["m_Item"];
            C_EconItemView.m_iItemDefinitionIndex = cs["C_EconItemView"]["fields"]["m_iItemDefinitionIndex"];

            C_CSPlayerPawn.m_entitySpottedState = cs["C_CSPlayerPawn"]["fields"]["m_entitySpottedState"];
            EntitySpottedState_t.m_bSpotted = cs["EntitySpottedState_t"]["fields"]["m_bSpotted"];

            return true;
        } catch (const std::exception& e) {
            printf("[!] Offset parse error: %s\n", e.what());
            return false;
        }
    }

    // x64 user space. 0xFFFFFFFFFFFFFFFF is a stale slot, not an entity list.
    static bool is_user_ptr(uintptr_t p) {
        return p >= 0x10000 && p <= 0x00007FFFFFFFFFFFULL;
    }

    TestResult functional_test() {
        uintptr_t client_base = g_memory->get_client_base();
        if (!client_base) {
            printf("[!] client_base is 0\n");
            return TestResult::OFFSETS_WRONG;
        }

        uintptr_t entity_slot = client_base + client.dwEntityList;
        uintptr_t entity_list = g_memory->read<uintptr_t>(entity_slot);
        if (entity_list != 0 && !is_user_ptr(entity_list)) {
            printf("[!] entity_list is not a user pointer (0x%llX, offset 0x%X)\n",
                   (unsigned long long)entity_list, client.dwEntityList);
            return TestResult::OFFSETS_WRONG;
        }

        const uint32_t hi_off = client.highestEntityIndex ? client.highestEntityIndex : 0x2120;
        auto sane_hi = [](int n) { return n > 0 && n < 16384; };
        auto system_ok = [&](uintptr_t obj) {
            if (!is_user_ptr(obj)) return false;
            if (!sane_hi(g_memory->read<int>(obj + hi_off))) return false;
            return is_user_ptr(g_memory->read<uintptr_t>(obj + 16));
        };
        if (!system_ok(entity_list)) {
            uintptr_t inner = entity_list ? g_memory->read<uintptr_t>(entity_list) : 0;
            if (system_ok(inner))
                entity_list = inner;
            else if (entity_list == 0 && system_ok(entity_slot))
                entity_list = entity_slot;
        }
        if (!entity_list || !system_ok(entity_list)) {
            // A null pointer is the entity system not being up yet (menu, loading,
            // or the first read racing attach). That is not proof the offsets are stale.
            if (!entity_list || entity_list < 0x10000)
                return TestResult::NOT_READY;
            printf("[!] entity system invalid (list 0x%llX, offset 0x%X)\n",
                   (unsigned long long)entity_list, client.dwEntityList);
            return TestResult::OFFSETS_WRONG;
        }

        uintptr_t first_page = g_memory->read<uintptr_t>(entity_list + 16);
        if (!is_user_ptr(first_page)) {
            if (!first_page)
                return TestResult::NO_PLAYERS;
            printf("[!] entity list page is not a user pointer (0x%llX)\n",
                   (unsigned long long)first_page);
            return TestResult::OFFSETS_WRONG;
        }

        int valid_players = 0;
        int bogus_reads = 0;

        for (int i = 1; i < 64; i++) {
            uintptr_t controller = g_memory->read<uintptr_t>(first_page + 112 * (i & 0x1FF));
            if (!controller) continue;

            uint32_t pawn_handle = g_memory->read<uint32_t>(
                controller + CCSPlayerController.m_hPawn);
            if (!pawn_handle) {
                pawn_handle = g_memory->read<uint32_t>(
                    controller + CCSPlayerController.m_hPlayerPawn);
            }
            if (!pawn_handle) continue;

            uintptr_t pawn_page = g_memory->read<uintptr_t>(
                entity_list + 8 * ((pawn_handle & 0x7FFF) >> 9) + 16);
            if (!pawn_page) continue;

            uintptr_t pawn = g_memory->read<uintptr_t>(
                pawn_page + 112 * (pawn_handle & 0x1FF));
            if (!pawn) continue;

            int team = g_memory->read<int>(pawn + C_BaseEntity.m_iTeamNum);
            int health = g_memory->read<int>(pawn + C_BaseEntity.m_iHealth);

            if (team >= 0 && team <= 3) {
                if (health >= 0 && health <= 100) {
                    valid_players++;
                } else if (health > 100 || health < -1) {
                    bogus_reads++;
                }
            } else {
                bogus_reads++;
            }
        }

        if (valid_players > 0 && bogus_reads <= valid_players) {
            printf("[+] Found %d valid players in functional test\n", valid_players);
            return TestResult::OK;
        }

        if (valid_players == 0 && bogus_reads == 0) {
            return TestResult::NO_PLAYERS;
        }

        printf("[!] Functional test failed: %d valid, %d bogus reads\n",
               valid_players, bogus_reads);
        return TestResult::OFFSETS_WRONG;
    }

    bool run_dumper() {
        const char* dumper = "cs2-dumper.exe";

        if (!std::filesystem::exists(dumper)) {
            printf("[!] %s not found in current directory\n", dumper);
            printf("[!] Download from https://github.com/a2x/cs2-dumper/releases\n");
            printf("[!] Place next to this executable and restart.\n");
            return false;
        }

        printf("[*] Running %s (最多等待 20 秒)...\n", dumper);

        STARTUPINFOA si{};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi{};
        char cmd[] = "cs2-dumper.exe";
        if (!CreateProcessA(nullptr, cmd, nullptr, nullptr, FALSE,
                            CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
            printf("[!] 无法启动 cs2-dumper (错误 %lu)\n", GetLastError());
            return false;
        }

        const DWORD wait = WaitForSingleObject(pi.hProcess, 20000);
        DWORD exit_code = 1;
        if (wait == WAIT_TIMEOUT) {
            printf("[!] cs2-dumper 超时，已结束该进程\n");
            TerminateProcess(pi.hProcess, 1);
        } else {
            GetExitCodeProcess(pi.hProcess, &exit_code);
        }
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        if (wait == WAIT_TIMEOUT || exit_code != 0) {
            printf("[!] cs2-dumper failed with code %lu\n", exit_code);
            return false;
        }

        const char* dump_dir = "output";
        if (!std::filesystem::exists(dump_dir)) {
            printf("[!] cs2-dumper output directory '%s' not found\n", dump_dir);
            return false;
        }

        std::filesystem::create_directories("offsets");

        struct FileCopy {
            const char* src;
            const char* dst;
        };
        static const FileCopy needed[] = {
            {"output/offsets.json",    "offsets/offsets.json"},
            {"output/client_dll.json", "offsets/client_dll.json"},
        };

        bool ok = true;
        for (const auto& fc : needed) {
            if (std::filesystem::exists(fc.src)) {
                try {
                    if (std::filesystem::exists(fc.dst))
                        std::filesystem::remove(fc.dst);
                    std::filesystem::copy_file(fc.src, fc.dst);
                    printf("[+] Copied %s -> %s\n", fc.src, fc.dst);
                } catch (const std::exception& e) {
                    printf("[!] Failed to copy %s: %s\n", fc.src, e.what());
                    ok = false;
                }
            } else {
                printf("[!] Expected file %s not found in dumper output\n", fc.src);
                ok = false;
            }
        }

        try {
            std::filesystem::remove_all(dump_dir);
            printf("[+] Cleaned up %s/\n", dump_dir);
        } catch (const std::exception& e) {
            printf("[!] Failed to clean up %s: %s\n", dump_dir, e.what());
        }

        return ok;
    }
};

inline Offsets g_offsets;