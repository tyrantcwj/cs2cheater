#include <Windows.h>
#include <cstdio>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <imgui.h>

#include "aimbot.h"
#include "types.h"
#include "settings.h"
#include "utils.h"
#include "offsets.h"
#include "memory/imemory.h"
#include "memory/memory_winapi.h"
#include "memory/memory_syscall.h"
#include "memory/memory_driver.h"
#include "memory/driver_manager.h"
#include "config.h"
#include "menu.h"
#include "crosshair.h"
#include "overlay.h"
#include "weapon_icons.h"
#include "entity_reader.h"
#include "visible_esp.h"
#include "spectators.h"
#include "radar.h"
#include "input/input.h"

static const char* CONFIG_PATH = "cs2esp.ini";
static volatile bool g_running = true;
static bool g_driver_backend_active = false;

// Spectator background update state
static std::atomic<bool> g_spectator_thread_running{false};
static std::thread g_spectator_thread;
static struct SpectatorSnapshot {
    uintptr_t entity_list = 0;
    uintptr_t local_pawn = 0;
    uintptr_t local_controller = 0;
    std::mutex mtx;
} g_spec_snapshot;

// Forward declarations for spectator thread functions
static void stop_spectator_thread();

static void save_and_exit() {
    Config::save(CONFIG_PATH);
    printf("[+] 配置已保存\n");
}

// 无论以何种方式退出都会执行的清理逻辑
static void cleanup_on_exit() {
    stop_aimbot_thread();
    stop_spectator_thread();
    // 关闭内存后端（若使用内核驱动则会卸载驱动）
    if (g_memory) {
        g_memory->close();
        g_memory.reset();
    }

    save_and_exit();
}

static BOOL WINAPI console_handler(DWORD event) {
    if (event == CTRL_C_EVENT || event == CTRL_CLOSE_EVENT ||
        event == CTRL_BREAK_EVENT || event == CTRL_LOGOFF_EVENT ||
        event == CTRL_SHUTDOWN_EVENT)
    {
        g_aimbot_running.store(false, std::memory_order_relaxed);
        g_running = false;

        cleanup_on_exit();
        return TRUE;
    }
    return FALSE;
}

// 崩溃处理：即使程序崩溃也尽量卸载内核驱动
static LONG WINAPI crash_handler(EXCEPTION_POINTERS* ex) {
    UNREFERENCED_PARAMETER(ex);
    printf("\n[!] 检测到崩溃，正在清理驱动...\n");

    // 直接强制清理驱动（此时 g_memory 可能已处于异常状态）
    if (g_driver_backend_active) {
        DriverManager::full_cleanup();
    }

    return EXCEPTION_CONTINUE_SEARCH; // 交给 Windows 继续处理崩溃
}

enum MemoryBackend {
    WinApi = 1,
    IndirectSyscall = 2,
    KernelDriver = 3
};

std::unique_ptr<IMemory> CreateMemoryBackend(MemoryBackend backend) {
    switch (backend) {
        case WinApi:           return std::make_unique<MemoryWinApi>();
        case IndirectSyscall:  return std::make_unique<MemorySyscall>();
        case KernelDriver:     return std::make_unique<MemoryDriver>();
        default:               throw std::runtime_error("无效的后端选项");
    }
}

// --- Spectator background updater ---
static void spectator_update_thread() {
    while (g_spectator_thread_running.load(std::memory_order_relaxed)) {
        uintptr_t el = 0, lp = 0, lc = 0;
        {
            std::lock_guard<std::mutex> lock(g_spec_snapshot.mtx);
            el = g_spec_snapshot.entity_list;
            lp = g_spec_snapshot.local_pawn;
            lc = g_spec_snapshot.local_controller;
        }

        if (el && lp) {
            g_spectators.update(el, lp, lc);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }
}

static void start_spectator_thread() {
    if (g_spectator_thread_running.exchange(true)) return;
    g_spectator_thread = std::thread(spectator_update_thread);
}

static void stop_spectator_thread() {
    if (!g_spectator_thread_running.exchange(false)) return;
    if (g_spectator_thread.joinable()) {
        g_spectator_thread.join();
    }
}
// --- end spectator updater ---

int main() {
    ensure_console();
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    set_working_directory_to_exe();

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    ImGui_ImplWin32_EnableDpiAwareness();

    // 首先注册各类退出/崩溃处理回调
    SetConsoleCtrlHandler(console_handler, TRUE);
    SetUnhandledExceptionFilter(crash_handler);
    std::atexit([]() {
        stop_aimbot_thread();
        if (g_driver_backend_active) {
            DriverManager::full_cleanup();
        }
    });

    if (g_config_loaded_from_disk = Config::load(CONFIG_PATH))
        printf("[+] 配置已加载\n");

    while (g_settings.memory_backend == -1) {
        printf("\n请选择内存读取后端：\n");
        printf("  1. 用户态 (WinAPI)               - 最简单，通用性最好\n");
        printf("  2. 用户态 (间接系统调用)         - 略隐蔽，绕过部分用户态 hook\n");
        printf("  3. 内核驱动 kdmapper (IOCTL)     - 需管理员权限及额外配置\n");
        printf("\n> ");

        int backend = -1;
        scanf_s("%d", &backend);
        if (backend < 1 || backend > 3) {
            printf("无效选项: %d\n", backend);
            continue;
        }

        if (backend == 3) {
            printf("\n");
            printf("=== 内核驱动 (kdmapper) 使用要求 ===\n");
            printf("  - 必须以管理员身份运行\n");
            printf("  - MemReaderKdmp.sys 与 kdmapper.exe 须与本程序在同一目录\n");
            printf("  - 需关闭「核心隔离」与「易受攻击驱动程序阻止列表」\n");
            printf("\n  驱动会被直接映射进内核内存。\n");
            printf("  映射后将一直驻留，直到重启电脑才会消失。\n");
            printf("\n  是否继续？(y/n): ");
            char c;
            scanf_s(" %c", &c, 1);
            if (c != 'y' && c != 'Y') {
                printf("已取消。\n");
                continue;
            }
        }


        g_settings.memory_backend = backend;
    }

    if (g_settings.memory_backend == KernelDriver && !is_running_as_admin()) {
        printf("[*] 模式 3 需要管理员权限，正在请求提权...\n");
        Config::save(CONFIG_PATH);
        if (!relaunch_as_admin()) {
            printf("[-] 提权失败或用户取消了 UAC。\n");
            printf("    请右键 iTyc-cs2.exe → 以管理员身份运行，再选模式 3。\n");
            pause_before_exit();
            CoUninitialize();
            return 1;
        }
        printf("[*] 已打开管理员窗口，请看新弹出的控制台。这个窗口可以关掉。\n");
        CoUninitialize();
        return 0;
    }

    // 记录是否使用内核驱动后端（供退出/崩溃清理使用）
    g_driver_backend_active = (g_settings.memory_backend == KernelDriver);

    // 创建内存后端并附加到 cs2.exe
    try {
        g_memory = CreateMemoryBackend(
            static_cast<MemoryBackend>(g_settings.memory_backend));
    }
    catch (const std::exception& e) {
        printf("[-] 创建内存后端失败: %s\n", e.what());
        pause_before_exit();
        CoUninitialize();
        return 1;
    }

    const std::wstring target_process = utf8_to_wide(g_settings.target_process);
    if (!g_memory->attach(target_process.c_str())) {
        printf("[-] 无法附加到 %s（请确认游戏已启动）。\n", g_settings.target_process);
        printf("[*] 建议：选 1 (WinAPI) 后端，或以管理员身份运行后再试。\n");

        if (g_driver_backend_active) {
            printf("[*] 正在清理驱动...\n");
            g_memory->close();
            g_memory.reset();
            g_driver_backend_active = false;
        }

        pause_before_exit();
        CoUninitialize();
        return 1;
    }

    printf("[+] 已附加到 %s (进程 ID: %lu)\n", g_settings.target_process, g_memory->get_pid());
    printf("[+] client.dll 基址: 0x%llX\n",
        (unsigned long long)g_memory->get_client_base());

    if (g_driver_backend_active) {
        printf("[+] 当前使用内核驱动读取内存\n");
    }

    if (!g_offsets.load("offsets/offsets.json", "offsets/client_dll.json")) {
        printf("[-] 加载偏移量失败\n");
        pause_before_exit();
        cleanup_on_exit();
        CoUninitialize();
        return 1;
    }

    if (!g_overlay.init(g_memory->get_pid(), g_settings.target_window)) {
        printf("[-] 叠加层 (Overlay) 初始化失败\n");
        pause_before_exit();
        cleanup_on_exit();
        CoUninitialize();
        return 1;
    }

    if (!g_input.initialize())
    {
        printf("[-] 输入注入模块初始化失败\n");
        pause_before_exit();
        cleanup_on_exit();
        CoUninitialize();
        return 1;
    }

    // Start background spectator updater (low frequency to reduce main-loop pressure)
    start_spectator_thread();

    if (g_settings.aimbot_enabled || g_settings.triggerbot_enabled) {
        start_aimbot_thread();
    }

    g_weapon_icons.init(g_overlay.get_device());

    printf("[+] %s = 菜单 | %s = 总开关 | %s = 退出\n",
           vk_name(g_settings.key_menu),
           vk_name(g_settings.key_master),
           vk_name(g_settings.key_exit));

    EntityReader entity_reader;
    bool prev_menu = g_settings.menu_open;
    bool was_aimbot_enabled = false;
    std::string last_map_name;

    g_overlay.set_interactive(g_settings.menu_open);

    while (g_running) {
        auto frame_start = std::chrono::high_resolution_clock::now();

        if (GetAsyncKeyState(g_settings.key_exit) & 1) break;

        if ((GetAsyncKeyState(g_settings.key_menu) & 1) && g_overlay.is_game_window()) g_menu.toggle();
        if (GetAsyncKeyState(g_settings.key_master) & 1)
            g_settings.master_switch = !g_settings.master_switch;

        if (g_settings.menu_open != prev_menu) {
            g_overlay.set_interactive(g_settings.menu_open);
            if (!g_settings.menu_open && prev_menu)
                Config::save(CONFIG_PATH);
            prev_menu = g_settings.menu_open;
        }

        bool want_aimbot = (g_settings.aimbot_enabled || g_settings.triggerbot_enabled) && g_settings.master_switch;
        if (want_aimbot && !was_aimbot_enabled) {
            start_aimbot_thread();
        }
        else if (!want_aimbot && was_aimbot_enabled) {
            stop_aimbot_thread();
        }
        was_aimbot_enabled = want_aimbot;

        if (!g_overlay.begin_frame()) break;
        g_menu.render();

        if (!g_settings.master_switch) {
            static constexpr int IDLE_FPS_CAP = 20;
            g_overlay.end_frame(0);
            limit_frame(frame_start, IDLE_FPS_CAP);
            continue;
        }

        FrameState state = entity_reader.read_frame(
            g_overlay.width, g_overlay.height);

        // Update spectator snapshot for background thread
        {
            std::lock_guard<std::mutex> lock(g_spec_snapshot.mtx);
            g_spec_snapshot.entity_list    = state.entity_list;
            g_spec_snapshot.local_pawn     = state.local.pawn;
            g_spec_snapshot.local_controller = state.local.controller;
        }

        if (g_settings.aimbot_enabled || g_settings.triggerbot_enabled)
        {
            AimbotFrame af{};
            af.view_matrix = state.view_matrix;
            af.local_x     = state.local.x;
            af.local_y     = state.local.y;
            af.local_z     = state.local.z;
            af.local_team  = state.local.team;
            af.local_pawn  = state.local.pawn;
            af.screen_w    = g_overlay.width;
            af.screen_h    = g_overlay.height;

            if (state.local.camera.valid)
            {
                af.eye_origin   = state.local.camera.origin;
                af.view_angles  = state.local.camera.angles;
                af.camera_fov   = state.local.camera.fov;
                af.camera_valid = true;
            }
            else
            {
                af.eye_origin  = { state.local.x,
                                   state.local.y,
                                   state.local.z + 64.0f };
                af.camera_fov  = 90.0f;
                af.camera_valid = false;
            }

            for (int i = 1; i < EntityList::MAX_PLAYERS; i++)
            {
                const auto& p = state.players[i];
                af.targets[i].valid      = p.valid;
                af.targets[i].team       = p.team;
                af.targets[i].health     = p.health;
                af.targets[i].head_pos   = p.head_world;
                af.targets[i].neck_pos   = p.neck_world;
                af.targets[i].chest_pos  = p.chest_world;
                af.targets[i].pelvis_pos = p.pelvis_world;
            }

            g_aimbot_data.publish(af);
        }

        if (state.local.observer_pawn != 0 && state.local.pawn != 0 && !state.map_name.empty() && state.map_name != "<empty>" && state.map_name != last_map_name) {
            printf( "[+] 地图切换: %s -> %s\n", last_map_name.data(), state.map_name.data());
            last_map_name = state.map_name;
            g_bvh.clear();
            printf( "[+] 正在解析地图 BVH: %s\n", last_map_name.data());
            g_bvh.parse();
            printf( "[+] BVH 解析完成\n" );
        }

        // Spectators are now updated in a background thread (see start_spectator_thread)

        ImDrawList* draw = ImGui::GetBackgroundDrawList();

        for (int i = 1; i < EntityList::MAX_PLAYERS; i++) {
            g_esp.draw_player(draw, state.players[i], state.local.team,
                g_overlay.width, g_overlay.height, i, state.local.is_scoped);
        }

        g_radar.draw(draw, state.radar_players, EntityList::MAX_PLAYERS,
            state.local.x, state.local.y, state.local.yaw, state.local.team,
            state.map_scale, g_overlay.width, g_overlay.height);

        g_spectators.draw(g_overlay.width);

        ImDrawList* fg = ImGui::GetForegroundDrawList();
        Crosshair::Config xhair_cfg = {
            g_settings.crosshair_enabled && g_settings.master_switch,
            g_settings.crosshair_shape,
            g_settings.crosshair_size,
            g_settings.crosshair_gap,
            g_settings.crosshair_thickness,
            float4_to_col(g_settings.crosshair_color),
            g_settings.crosshair_outline,
            g_settings.crosshair_outline_thickness,
            float4_to_col(g_settings.crosshair_outline_color),
            g_settings.crosshair_dot,
            g_settings.crosshair_dot_size,
        };
        g_crosshair.draw(fg, g_overlay.width, g_overlay.height, xhair_cfg);

        g_overlay.end_frame(g_settings.use_vsync ? 1 : 0);

        if (!g_settings.use_vsync) {
            limit_frame(frame_start, g_settings.target_fps);
        }
    }

    // 正常退出
    printf("\n[*] 正在关闭...\n");
    g_weapon_icons.shutdown();
    g_overlay.shutdown();
    cleanup_on_exit();
    CoUninitialize();
    return 0;
}