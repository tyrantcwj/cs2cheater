#pragma once
#include <chrono>
#include <cstdio>
#include <thread>
#include <string>

#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "shell32.lib")

inline std::wstring exe_directory() {
    wchar_t path[MAX_PATH]{};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring dir(path);
    const size_t slash = dir.find_last_of(L"\\/");
    if (slash != std::wstring::npos)
        dir.resize(slash);
    return dir;
}

inline void set_working_directory_to_exe() {
    const std::wstring dir = exe_directory();
    if (!dir.empty())
        SetCurrentDirectoryW(dir.c_str());
}

inline bool is_running_as_admin() {
    BOOL is_admin = FALSE;
    SID_IDENTIFIER_AUTHORITY auth = SECURITY_NT_AUTHORITY;
    PSID admin_group = nullptr;
    if (AllocateAndInitializeSid(&auth, 2,
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &admin_group)) {
        CheckTokenMembership(nullptr, admin_group, &is_admin);
        FreeSid(admin_group);
    }
    return is_admin != FALSE;
}

inline void ensure_console() {
    if (!GetConsoleWindow())
        AllocConsole();
    FILE* dummy = nullptr;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
    freopen_s(&dummy, "CONIN$", "r", stdin);
    SetConsoleTitleW(L"iTyc-cs2");
    setvbuf(stdout, nullptr, _IONBF, 0);
}

inline void pause_before_exit() {
    printf("\n按回车关闭窗口...\n");
    fflush(stdout);
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    if (!in || in == INVALID_HANDLE_VALUE) return;
    FlushConsoleInputBuffer(in);
    for (;;) {
        INPUT_RECORD rec{};
        DWORD read = 0;
        if (!ReadConsoleInputW(in, &rec, 1, &read) || read == 0)
            break;
        if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown &&
            rec.Event.KeyEvent.wVirtualKeyCode == VK_RETURN)
            break;
    }
}

inline bool relaunch_as_admin() {
    wchar_t path[MAX_PATH]{};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    const std::wstring dir = exe_directory();
    const HINSTANCE r = ShellExecuteW(nullptr, L"runas", path, nullptr,
                                      dir.empty() ? nullptr : dir.c_str(),
                                      SW_SHOWNORMAL);
    return reinterpret_cast<INT_PTR>(r) > 32;
}

inline std::wstring utf8_to_wide(const char* text) {
    if (!text || !*text) return {};
    const int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    if (len <= 0) return {};
    std::wstring out(static_cast<size_t>(len - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text, -1, out.data(), len);
    return out;
}

struct FindGameWindowResult {
    HWND  hwnd = nullptr;
    DWORD pid  = 0;
};

inline bool hwnd_belongs_to_process(HWND hwnd, DWORD pid) {
    if (!hwnd || !pid) return false;
    DWORD wpid = 0;
    GetWindowThreadProcessId(hwnd, &wpid);
    return wpid == pid;
}

inline bool hwnd_client_size(HWND hwnd, int* out_w, int* out_h) {
    if (!hwnd) return false;
    RECT rc{};
    if (!GetClientRect(hwnd, &rc)) return false;
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    if (w <= 0 || h <= 0) {
        // Exclusive fullscreen / mode switch can briefly report a zero client.
        // Window rect is still usable for picking the real game surface.
        RECT wr{};
        if (!GetWindowRect(hwnd, &wr)) return false;
        w = wr.right - wr.left;
        h = wr.bottom - wr.top;
    }
    if (out_w) *out_w = w;
    if (out_h) *out_h = h;
    return w >= 320 && h >= 240;
}

inline FindGameWindowResult find_game_window(DWORD pid, const wchar_t* preferred_title) {
    FindGameWindowResult result{};
    result.pid = pid;

    auto find_by_pid = [&]() -> HWND {
        if (!pid) return nullptr;

        struct EnumCtx {
            DWORD pid;
            HWND  best      = nullptr;
            int   best_area = 0;
            HWND  sdl_best  = nullptr;
            int   sdl_area  = 0;
        } ctx{ pid };

        EnumWindows([](HWND hwnd, LPARAM lp) -> BOOL {
            auto* c = reinterpret_cast<EnumCtx*>(lp);
            DWORD wpid = 0;
            GetWindowThreadProcessId(hwnd, &wpid);
            if (wpid != c->pid)
                return TRUE;
            if (!IsWindowVisible(hwnd) || IsIconic(hwnd))
                return TRUE;

            wchar_t cls[128]{};
            GetClassNameW(hwnd, cls, 128);
            const bool is_sdl = (cls[0] && _wcsicmp(cls, L"SDL_app") == 0);

            wchar_t title[256]{};
            GetWindowTextW(hwnd, title, 256);
            // CS2's real surface is SDL_app; allow empty title only for that class.
            if (!title[0] && !is_sdl)
                return TRUE;

            int w = 0, h = 0;
            if (!hwnd_client_size(hwnd, &w, &h))
                return TRUE;

            const int area = w * h;
            if (area > c->best_area) {
                c->best_area = area;
                c->best      = hwnd;
            }
            if (is_sdl && area > c->sdl_area) {
                c->sdl_area = area;
                c->sdl_best = hwnd;
            }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&ctx));

        if (ctx.sdl_best) return ctx.sdl_best;
        return ctx.best;
    };

    auto try_title = [&](const wchar_t* title) -> HWND {
        if (!title || !*title) return nullptr;
        HWND found = FindWindowW(nullptr, title);
        if (!found) return nullptr;
        if (pid && !hwnd_belongs_to_process(found, pid))
            return nullptr;
        if (!IsWindowVisible(found) || IsIconic(found))
            return nullptr;
        int w = 0, h = 0;
        if (!hwnd_client_size(found, &w, &h))
            return nullptr;
        return found;
    };

    static const wchar_t* kFallbackTitles[] = {
        L"Counter-Strike 2",
        L"反恐精英：全球攻势",
        L"反恐精英:全球攻势",
        L"Counter-Strike: Global Offensive",
        L"反恐精英 2",
    };

    // Prefer the live SDL surface owned by the attached process.
    if (HWND hwnd = find_by_pid())
        return { hwnd, pid };

    if (preferred_title && *preferred_title) {
        if (HWND hwnd = try_title(preferred_title))
            return { hwnd, pid };
    }

    for (const wchar_t* title : kFallbackTitles) {
        if (preferred_title && *preferred_title && wcscmp(title, preferred_title) == 0)
            continue;
        if (HWND hwnd = try_title(title))
            return { hwnd, pid };
    }

    return result;
}

inline void limit_frame(std::chrono::high_resolution_clock::time_point frame_start,
                        double target_fps) {
    using namespace std::chrono;

    static bool timer_initialized = false;
    if (!timer_initialized) {
        timeBeginPeriod(1);
        timer_initialized = true;
    }

    long long target_ns = static_cast<long long>(1'000'000'000.0 / target_fps);
    nanoseconds target(target_ns);

    nanoseconds elapsed = high_resolution_clock::now() - frame_start;
    nanoseconds remaining = target - elapsed;

    if (remaining > milliseconds(2)) {
        nanoseconds sleep_time = remaining - microseconds(1500);
        std::this_thread::sleep_for(sleep_time);
    }

    while (high_resolution_clock::now() - frame_start < target) {
        _mm_pause();
    }
}

inline ImU32 apply_opacity(ImU32 col, float opacity) {
    if (opacity >= 1.0f) return col;
    ImU32 a = (ImU32)(((col >> 24) & 0xFF) * opacity);
    return (col & 0x00FFFFFF) | (a << 24);
}

inline const char* vk_name(int vk) {
    switch (vk) {
    case VK_LBUTTON: return "Mouse1";
    case VK_RBUTTON: return "Mouse2";
    case VK_MBUTTON: return "Mouse3";
    case VK_XBUTTON1: return "Mouse4";
    case VK_XBUTTON2: return "Mouse5";
    case VK_LSHIFT: return "LShift";
    case VK_RSHIFT: return "RShift";
    case VK_LCONTROL: return "LCtrl";
    case VK_RCONTROL: return "RCtrl";
    case VK_LMENU: return "LAlt";
    case VK_RMENU: return "RAlt";
    case VK_CAPITAL: return "CapsLock";
    case VK_TAB: return "Tab";
    case VK_SPACE: return "Space";
    case VK_F1: return "F1";
    case VK_F2: return "F2";
    case VK_F3: return "F3";
    case VK_F4: return "F4";
    case VK_F5: return "F5";
    case VK_F6: return "F6";
    case VK_F7: return "F7";
    case VK_F8: return "F8";
    case VK_F9: return "F9";
    case VK_F10: return "F10";
    case VK_F11: return "F11";
    case VK_F12: return "F12";
    case VK_NUMPAD1: return "Num1";
    case VK_NUMPAD2: return "Num2";
    case VK_NUMPAD3: return "Num3";
    case VK_NUMPAD4: return "Num4";
    case VK_NUMPAD5: return "Num5";
    case VK_NUMPAD6: return "Num6";
    case VK_NUMPAD7: return "Num7";
    case VK_NUMPAD8: return "Num8";
    case VK_NUMPAD9: return "Num9";
    case VK_NUMPAD0: return "Num0";
    case VK_INSERT: return "Insert";
    case VK_DELETE: return "Delete";
    case VK_HOME: return "Home";
    case VK_END: return "End";
    case VK_PRIOR: return "PageUp";
    case VK_NEXT: return "PageDown";
    case VK_ESCAPE: return "Escape";
    default: {
        static char buf[32];
        UINT scan = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);
        if (GetKeyNameTextA(scan << 16, buf, 32)) return buf;
        snprintf(buf, 32, "Key 0x%02X", vk);
        return buf;
    }
    }
}

inline int scan_any_key(bool allow_mouse1) {
    static bool prev_state[256] = {};

    // Check Escape first
    bool esc_down = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
    if (esc_down && !prev_state[VK_ESCAPE]) {
        // Update all states before returning
        for (int k = 1; k < 256; ++k)
            prev_state[k] = (GetAsyncKeyState(k) & 0x8000) != 0;
        return -1;
    }

    int result = 0;

    for (int k = 1; k < 256; ++k)
    {
        bool is_down = (GetAsyncKeyState(k) & 0x8000) != 0;
        bool was_down = prev_state[k];

        if (k == VK_ESCAPE) {
            prev_state[k] = is_down;
            continue;
        }

        if (!allow_mouse1 && k == VK_LBUTTON) {
            prev_state[k] = is_down;
            continue;
        }

        // Skip modifier keys - you probably don't want to bind ctrl/shift/alt alone
        if (k == VK_SHIFT || k == VK_CONTROL || k == VK_MENU ||
            k == VK_LSHIFT || k == VK_RSHIFT ||
            k == VK_LCONTROL || k == VK_RCONTROL ||
            k == VK_LMENU || k == VK_RMENU) {
            prev_state[k] = is_down;
            continue;
            }

        // Detect fresh press (edge: not down before, down now)
        if (is_down && !was_down && result == 0) {
            result = k;
        }

        prev_state[k] = is_down;
    }

    return result;
}