// driver_manager.h - corrected check functions

#pragma once
#include <Windows.h>
#include <string>
#include <cstdio>
#include <vector>
#include <filesystem>
#include "shared.h"

class DriverManager {
public:
    struct SystemStatus {
        bool secure_boot_enabled;
        bool test_signing_enabled;
        bool driver_file_exists;
        bool driver_already_loaded;
        bool is_admin;
        bool vulnerable_driver_blocklist_enabled;
        bool memory_integrity_enabled;
    };

    static SystemStatus check_system() {
        SystemStatus s{};
        s.is_admin = check_admin();
        s.secure_boot_enabled = check_secure_boot();
        s.test_signing_enabled = check_test_signing();
        s.driver_file_exists = check_driver_file();
        s.driver_already_loaded = check_driver_loaded();
        s.vulnerable_driver_blocklist_enabled = check_vulnerable_driver_blocklist();
        s.memory_integrity_enabled            = check_memory_integrity();
        return s;
    }

    static void print_status(const SystemStatus& s) {
        printf("\n=== Kernel Driver System Check ===\n");
        printf("  Admin privileges:         %s\n", s.is_admin              ? "YES"              : "NO (REQUIRED)");
        printf("  Secure Boot:              %s\n", s.secure_boot_enabled   ? "ON (disable BIOS)" : "OFF (good)");
        printf("  Test Signing:             %s\n", s.test_signing_enabled  ? "ENABLED (good)"   : "DISABLED");
        printf("  Driver file:              %s\n", s.driver_file_exists    ? "FOUND"             : "MISSING");
        printf("  Driver loaded:            %s\n", s.driver_already_loaded ? "YES"               : "NO");
        printf("  Driver Blocklist:         %s\n", s.vulnerable_driver_blocklist_enabled ? "ENABLED (will disable)" : "OFF (good)");
        printf("  Memory Integrity (HVCI):  %s\n", s.memory_integrity_enabled            ? "ENABLED (will disable)" : "OFF (good)");
        printf("==================================\n\n");
    }
    enum SetupResult {
        READY,
        NEED_REBOOT,
        NEED_BIOS,
        NEED_ADMIN,
        DRIVER_FILE_MISSING,
        SETUP_FAILED
    };

    static bool is_admin() { return check_admin(); }

    static SetupResult setup_kdmapper() {
        printf("\n=== 内核驱动 (kdmapper) 检查 ===\n");
        printf("  管理员权限:     %s\n", check_admin() ? "是" : "否");
        printf("  驱动文件:       %s\n", check_file_exists(get_driver_path(true)) ? "找到" : "缺失");
        printf("  kdmapper.exe:   %s\n", check_file_exists(get_kdmapper_path()) ? "找到" : "缺失");
        printf("  驱动拦截列表:   %s\n", check_vulnerable_driver_blocklist() ? "开启" : "关闭");
        printf("  内存完整性:     %s\n", check_memory_integrity() ? "开启" : "关闭");
        printf("================================\n\n");

        if (!check_admin()) {
            printf("[-] 必须以管理员身份运行。\n");
            return NEED_ADMIN;
        }

        if (!check_file_exists(get_driver_path(true))) {
            printf("[-] 未找到 MemReaderKdmp.sys（须与程序同一目录）。\n");
            printf("    路径: %ls\n", get_driver_path(true).c_str());
            return DRIVER_FILE_MISSING;
        }
        if (!check_file_exists(get_kdmapper_path())) {
            printf("[-] 未找到 kdmapper.exe（须与程序同一目录）。\n");
            printf("    路径: %ls\n", get_kdmapper_path().c_str());
            return DRIVER_FILE_MISSING;
        }

        bool need_reboot = false;

        // -- Vulnerable Driver Blocklist --
        if (check_vulnerable_driver_blocklist()) {
            printf("[*] Vulnerable Driver Blocklist is enabled. Disabling...\n");
            if (disable_vulnerable_driver_blocklist()) {
                printf("[+] Disabled. Reboot required.\n");
                need_reboot = true;
            } else {
                printf("[-] Failed to disable Vulnerable Driver Blocklist.\n");
                return SETUP_FAILED;
            }
        }

        // -- Memory Integrity (HVCI) --
        if (check_memory_integrity()) {
            printf("[*] Memory Integrity (HVCI) is enabled. Disabling...\n");
            if (disable_memory_integrity()) {
                printf("[+] Disabled. Reboot required.\n");
                need_reboot = true;
            } else {
                printf("[-] Failed to disable Memory Integrity.\n");
                return SETUP_FAILED;
            }
        }

        if (need_reboot) {
            printf("\n[!] Registry changes applied. A REBOOT is required.\n");
            printf("    After reboot, run this program again.\n");
            printf("\n    Reboot now? (y/n): ");
            char c;
            scanf_s(" %c", &c, 1);
            if (c == 'y' || c == 'Y')
                system("shutdown /r /t 3 /c \"Rebooting to apply kdmapper requirements\"");
            return NEED_REBOOT;
        }

        if (ping_driver(true)) {
            printf("[+] 驱动已加载并响应。\n");
            return READY;
        }

        printf("[*] 正在用 kdmapper 映射驱动...\n");
        if (!run_kdmapper())
            return SETUP_FAILED;

        if (!ping_driver_retry(true, 12, 250)) {
            printf("[-] 驱动已映射，但对 ping 无响应。\n");
            printf("    常见原因：核心隔离仍开启、拦截列表仍生效、需重启后再试。\n");
            printf("    CreateFile 错误码: %lu\n", GetLastError());
            return SETUP_FAILED;
        }

        printf("[+] 驱动已映射并响应。\n");
        return READY;
    }

    static SetupResult setup() {
        SystemStatus s = check_system();
        print_status(s);

        if (!s.is_admin) {
            printf("[-] Must run as Administrator for kernel driver.\n");
            printf("    Right-click the .exe -> Run as administrator\n");
            return NEED_ADMIN;
        }

        if (!s.driver_file_exists) {
            printf("[-] Driver file not found: %ls\n", get_driver_path().c_str());
            printf("    Place MemReader.sys next to this executable.\n");
            return DRIVER_FILE_MISSING;
        }

        if (s.secure_boot_enabled) {
            printf("[-] Secure Boot is ENABLED.\n");
            printf("    You must disable it in your BIOS/UEFI settings:\n");
            printf("    1. Restart your PC\n");
            printf("    2. Enter BIOS (press DEL, F2, or F12 during boot)\n");
            printf("    3. Find Security > Secure Boot > Disable\n");
            printf("    4. Save & Exit\n");
            printf("    5. Run this program again\n");
            return NEED_BIOS;
        }

        if (!s.test_signing_enabled) {
            printf("[*] Test signing is not enabled. Enabling now...\n");

            if (enable_test_signing()) {
                printf("[+] Test signing enabled successfully.\n");
                printf("[!] YOU MUST REBOOT for this to take effect.\n");
                printf("    After reboot, run this program again.\n");
                printf("\n    Reboot now? (y/n): ");

                char c;
                scanf_s(" %c", &c, 1);
                if (c == 'y' || c == 'Y') {
                    system("shutdown /r /t 5 /c \"Rebooting to enable test signing for kernel driver\"");
                }
                return NEED_REBOOT;
            } else {
                printf("[-] Failed to enable test signing.\n");
                return SETUP_FAILED;
            }
        }

        return READY;
    }

    static bool load_driver() {
        std::wstring path = get_driver_path();

        if (check_driver_loaded()) {
            printf("[*] Driver already loaded, verifying...\n");
            if (ping_driver()) {
                printf("[+] Driver responding.\n");
                return true;
            }
            printf("[*] Driver not responding, reloading...\n");
            unload_driver();
        }

        printf("[*] Loading driver from: %ls\n", path.c_str());

        SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
        if (!scm) {
            printf("[-] OpenSCManager failed: %lu\n", GetLastError());
            return false;
        }

        SC_HANDLE service = OpenServiceW(scm, DRIVER_SERVICE_NAME, SERVICE_ALL_ACCESS);
        if (service) {
            SERVICE_STATUS ss;
            ControlService(service, SERVICE_CONTROL_STOP, &ss);
            DeleteService(service);
            CloseServiceHandle(service);
            Sleep(500);
            service = nullptr;
        }

        service = CreateServiceW(
            scm, DRIVER_SERVICE_NAME, DRIVER_SERVICE_NAME,
            SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
            path.c_str(),
            nullptr, nullptr, nullptr, nullptr, nullptr
        );

        if (!service) {
            DWORD err = GetLastError();
            if (err == ERROR_SERVICE_EXISTS) {
                service = OpenServiceW(scm, DRIVER_SERVICE_NAME, SERVICE_ALL_ACCESS);
            }
            if (!service) {
                printf("[-] CreateService failed: %lu\n", err);
                CloseServiceHandle(scm);
                return false;
            }
        }

        if (!StartServiceW(service, 0, nullptr)) {
            DWORD err = GetLastError();
            if (err != ERROR_SERVICE_ALREADY_RUNNING) {
                printf("[-] StartService failed: %lu\n", err);
                if (err == ERROR_INVALID_IMAGE_HASH) {
                    printf("    Driver is not properly signed.\n");
                    printf("    Make sure test signing is enabled and you've rebooted.\n");
                }
                DeleteService(service);
                CloseServiceHandle(service);
                CloseServiceHandle(scm);
                return false;
            }
        }

        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        Sleep(200);

        if (!ping_driver()) {
            printf("[-] Driver loaded but not responding.\n");
            unload_driver();
            return false;
        }

        printf("[+] Driver loaded and responding.\n");
        return true;
    }

    static bool unload_driver() {
        printf("[*] Unloading driver...\n");

        SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
        if (!scm) return false;

        SC_HANDLE service = OpenServiceW(scm, DRIVER_SERVICE_NAME, SERVICE_ALL_ACCESS);
        if (!service) {
            CloseServiceHandle(scm);
            return true;
        }

        SERVICE_STATUS status;
        ControlService(service, SERVICE_CONTROL_STOP, &status);

        int retries = 10;
        while (retries-- > 0) {
            if (QueryServiceStatus(service, &status) &&
                status.dwCurrentState == SERVICE_STOPPED) {
                break;
            }
            Sleep(200);
        }

        BOOL deleted = DeleteService(service);
        CloseServiceHandle(service);
        CloseServiceHandle(scm);

        if (deleted) {
            printf("[+] Driver unloaded and service deleted.\n");
        } else {
            printf("[!] Driver stopped, service deletion pending.\n");
        }

        return deleted != FALSE;
    }

    static void full_cleanup() {
        unload_driver();
        if (!check_driver_loaded()) {
            printf("[+] Cleanup complete. No driver traces in system.\n");
        }
    }

private:
    static bool run_kdmapper() {
        const std::wstring mapper = get_kdmapper_path();
        const std::wstring sys    = get_driver_path(true);
        const std::wstring dir    = mapper.substr(0, mapper.find_last_of(L"\\/"));
        std::wstring cmd = L"\"" + mapper + L"\" \"" + sys + L"\"";
        std::vector<wchar_t> buf(cmd.begin(), cmd.end());
        buf.push_back(L'\0');

        STARTUPINFOW si{};
        PROCESS_INFORMATION pi{};
        si.cb = sizeof(si);

        if (!CreateProcessW(
                nullptr,
                buf.data(),
                nullptr, nullptr,
                TRUE,
                0,
                nullptr,
                dir.empty() ? nullptr : dir.c_str(),
                &si, &pi))
        {
            printf("[-] 启动 kdmapper 失败: %lu\n", GetLastError());
            return false;
        }

        const DWORD wait = WaitForSingleObject(pi.hProcess, 30000);
        DWORD exit_code = 1;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (wait == WAIT_TIMEOUT) {
            printf("[-] kdmapper 超时（30 秒）。\n");
            return false;
        }
        if (exit_code == STILL_ACTIVE) {
            printf("[-] kdmapper 仍在运行，未能完成映射。\n");
            return false;
        }
        if (exit_code != 0) {
            printf("[-] kdmapper 退出码: %lu\n", exit_code);
            printf("    若提示 HVCI / blocklist / 0xC0000142，请关闭核心隔离后重启。\n");
            return false;
        }

        return true;
    }

    static bool check_file_exists(const std::wstring& path) {
        std::error_code ec;
        return std::filesystem::exists(path, ec);
    }

    static std::wstring get_kdmapper_path() {
        wchar_t exe_path[MAX_PATH];
        GetModuleFileNameW(nullptr, exe_path, MAX_PATH);
        std::wstring path(exe_path);
        size_t last_slash = path.find_last_of(L"\\/");
        if (last_slash != std::wstring::npos)
            path = path.substr(0, last_slash + 1);
        path += L"kdmapper.exe";
        return path;
    }

    // ================================================================
    // Vulnerable Driver Blocklist check
    // ================================================================
    static bool check_vulnerable_driver_blocklist() {
        HKEY hKey = nullptr;
        LONG result = RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\CI\\Config",
            0, KEY_READ, &hKey
        );
        if (result != ERROR_SUCCESS)
            return true; // Win10/11 默认开启拦截列表

        DWORD value = 1;
        DWORD size = sizeof(value);
        const LONG q = RegQueryValueExW(hKey, L"VulnerableDriverBlocklistEnable",
                         nullptr, nullptr, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
        if (q != ERROR_SUCCESS) return true;
        return value != 0;
    }

    static bool disable_vulnerable_driver_blocklist() {
        HKEY hKey = nullptr;
        LONG result = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\CI\\Config",
            0, nullptr, 0, KEY_SET_VALUE, nullptr, &hKey, nullptr
        );
        if (result != ERROR_SUCCESS) return false;

        DWORD value = 0;
        result = RegSetValueExW(hKey, L"VulnerableDriverBlocklistEnable",
                                0, REG_DWORD, (LPBYTE)&value, sizeof(value));
        RegCloseKey(hKey);
        return result == ERROR_SUCCESS;
    }

    // ================================================================
    // Memory Integrity (HVCI) check
    // ================================================================
    static bool check_hvci_runtime() {
        typedef struct _SYSTEM_CODEINTEGRITY_INFORMATION {
            ULONG Length;
            ULONG CodeIntegrityOptions;
        } SYSTEM_CODEINTEGRITY_INFORMATION;

        constexpr ULONG CODEINTEGRITY_OPTION_HVCI_KMCI_ENABLED = 0x400;
        constexpr ULONG SystemCodeIntegrityInformation = 103;

        using NtQuerySystemInformation_t = LONG (WINAPI*)(ULONG, PVOID, ULONG, PULONG);
        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (!ntdll) return false;
        auto NtQuerySystemInformation = reinterpret_cast<NtQuerySystemInformation_t>(
            GetProcAddress(ntdll, "NtQuerySystemInformation"));
        if (!NtQuerySystemInformation) return false;

        SYSTEM_CODEINTEGRITY_INFORMATION info{};
        info.Length = sizeof(info);
        if (NtQuerySystemInformation(SystemCodeIntegrityInformation, &info, sizeof(info), nullptr) != 0)
            return false;
        return (info.CodeIntegrityOptions & CODEINTEGRITY_OPTION_HVCI_KMCI_ENABLED) != 0;
    }

    static bool check_memory_integrity() {
        if (check_hvci_runtime())
            return true;

        HKEY hKey = nullptr;
        LONG result = RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\DeviceGuard\\"
            L"Scenarios\\HypervisorEnforcedCodeIntegrity",
            0, KEY_READ, &hKey
        );
        if (result != ERROR_SUCCESS) return false;

        DWORD value = 0;
        DWORD size = sizeof(value);
        RegQueryValueExW(hKey, L"Enabled",
                         nullptr, nullptr, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
        return value == 1;
    }

    static bool disable_memory_integrity() {
        HKEY hKey = nullptr;
        LONG result = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\DeviceGuard\\"
            L"Scenarios\\HypervisorEnforcedCodeIntegrity",
            0, nullptr, 0, KEY_SET_VALUE, nullptr, &hKey, nullptr
        );
        if (result != ERROR_SUCCESS) return false;

        DWORD value = 0;
        result = RegSetValueExW(hKey, L"Enabled",
                                0, REG_DWORD, (LPBYTE)&value, sizeof(value));
        RegCloseKey(hKey);
        return result == ERROR_SUCCESS;
    }

    // ================================================================
    // FIXED: Secure Boot check using registry
    // ================================================================
    static bool check_secure_boot() {
        HKEY hKey = nullptr;
        LONG result = RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\SecureBoot\\State",
            0,
            KEY_READ,
            &hKey
        );

        if (result != ERROR_SUCCESS) {
            // Key doesn't exist:
            //   - Legacy BIOS (no UEFI, no Secure Boot concept)
            //   - Or Secure Boot completely absent
            // Either way, no Secure Boot blocking us
            return false;
        }

        DWORD value = 0;
        DWORD size = sizeof(value);
        DWORD type = 0;

        result = RegQueryValueExW(
            hKey,
            L"UEFISecureBootEnabled",
            nullptr,
            &type,
            reinterpret_cast<LPBYTE>(&value),
            &size
        );

        RegCloseKey(hKey);

        if (result != ERROR_SUCCESS) {
            // Value doesn't exist within the key
            // Unusual but means Secure Boot state is indeterminate
            // Assume not enabled
            return false;
        }

        return value == 1;
    }

    // ================================================================
    // FIXED: Test signing check using registry (not bcdedit parsing)
    // ================================================================
    static bool check_test_signing() {
        // Method: Use NtQuerySystemInformation with SystemCodeIntegrityInformation
        // This tells us the ACTUAL runtime enforcement state

        typedef struct _SYSTEM_CODEINTEGRITY_INFORMATION {
            ULONG Length;
            ULONG CodeIntegrityOptions;
        } SYSTEM_CODEINTEGRITY_INFORMATION;

        // CodeIntegrityOptions flags
        const ULONG CODEINTEGRITY_OPTION_TESTSIGN = 0x02;

        typedef LONG(WINAPI* NtQuerySystemInformation_t)(
            ULONG SystemInformationClass,
            PVOID SystemInformation,
            ULONG SystemInformationLength,
            PULONG ReturnLength
        );

        const ULONG SystemCodeIntegrityInformation = 103;

        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (!ntdll) return false;

        auto NtQuerySystemInformation =
            reinterpret_cast<NtQuerySystemInformation_t>(
                GetProcAddress(ntdll, "NtQuerySystemInformation")
            );

        if (!NtQuerySystemInformation) return false;

        SYSTEM_CODEINTEGRITY_INFORMATION info{};
        info.Length = sizeof(info);

        LONG status = NtQuerySystemInformation(
            SystemCodeIntegrityInformation,
            &info,
            sizeof(info),
            nullptr
        );

        if (status != 0) {
            // Fallback: try bcdedit parsing
            return check_test_signing_bcdedit();
        }

        // Check if test signing flag is set
        return (info.CodeIntegrityOptions & CODEINTEGRITY_OPTION_TESTSIGN) != 0;
    }

    // Fallback bcdedit check
    static bool check_test_signing_bcdedit() {
        FILE* pipe = _popen("bcdedit /enum {current} 2>&1", "r");
        if (!pipe) return false;

        char buffer[512];
        bool found = false;

        while (fgets(buffer, sizeof(buffer), pipe)) {
            // Look for line containing "testsigning" and "Yes"
            std::string line(buffer);

            // Convert to lowercase for reliable matching
            for (auto& c : line) c = (char)tolower((unsigned char)c);

            if (line.find("testsigning") != std::string::npos &&
                line.find("yes") != std::string::npos)
            {
                found = true;
                break;
            }
        }

        _pclose(pipe);
        return found;
    }

    static bool check_admin() {
        BOOL is_admin = FALSE;
        SID_IDENTIFIER_AUTHORITY auth = SECURITY_NT_AUTHORITY;
        PSID admin_group = nullptr;

        if (AllocateAndInitializeSid(
                &auth, 2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &admin_group))
        {
            CheckTokenMembership(nullptr, admin_group, &is_admin);
            FreeSid(admin_group);
        }
        return is_admin != FALSE;
    }

    static bool check_driver_file() {
        std::error_code ec;
        return std::filesystem::exists(get_driver_path(), ec);
    }

    static bool check_driver_loaded() {
        SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (!scm) return false;

        SC_HANDLE service = OpenServiceW(scm, DRIVER_SERVICE_NAME, SERVICE_QUERY_STATUS);
        if (!service) {
            CloseServiceHandle(scm);
            return false;
        }

        SERVICE_STATUS status;
        BOOL ok = QueryServiceStatus(service, &status);

        CloseServiceHandle(service);
        CloseServiceHandle(scm);

        return ok && status.dwCurrentState == SERVICE_RUNNING;
    }

    static bool enable_test_signing() {
        // Run bcdedit and verify by checking the actual state after
        system("bcdedit /set testsigning on >nul 2>&1");

        // Can't verify immediately because it requires reboot
        // to take effect. But we can verify bcdedit accepted it
        // by reading BCD store.

        // Simple verification: run bcdedit again and check output
        FILE* pipe = _popen("bcdedit /enum {current} 2>&1", "r");
        if (!pipe) return false;

        char buffer[512];
        bool found = false;

        while (fgets(buffer, sizeof(buffer), pipe)) {
            std::string line(buffer);
            for (auto& c : line) c = (char)tolower((unsigned char)c);

            if (line.find("testsigning") != std::string::npos &&
                line.find("yes") != std::string::npos)
            {
                found = true;
                break;
            }
        }

        _pclose(pipe);
        return found;
    }

    static bool ping_driver(bool kdmapper = false) {
        HANDLE h = CreateFileW(
            kdmapper ? KDMP_USER_PATH : DRIVER_USER_PATH,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_EXISTING, 0, nullptr
        );
        if (h == INVALID_HANDLE_VALUE) return false;

        PING_RESPONSE resp{};
        DWORD returned = 0;
        BOOL ok = DeviceIoControl(h, IOCTL_PING,
            nullptr, 0, &resp, sizeof(resp), &returned, nullptr);
        CloseHandle(h);
        return ok && returned == sizeof(PING_RESPONSE)
                  && resp.magic == PING_MAGIC;
    }

    static bool ping_driver_retry(bool kdmapper, int attempts, int delay_ms) {
        for (int i = 0; i < attempts; ++i) {
            if (ping_driver(kdmapper))
                return true;
            Sleep(delay_ms);
        }
        return false;
    }

    static std::wstring get_driver_path(bool kdmapper = false) {
        wchar_t exe_path[MAX_PATH];
        GetModuleFileNameW(nullptr, exe_path, MAX_PATH);
        std::wstring path(exe_path);
        size_t last_slash = path.find_last_of(L"\\/");
        if (last_slash != std::wstring::npos)
            path = path.substr(0, last_slash + 1);
        path += kdmapper ? KDMP_FILE_NAME : DRIVER_FILE_NAME;
        return path;
    }
};