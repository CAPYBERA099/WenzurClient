/*
 *  WenzGuard v3.0 — Session Security Monitor
 *
 *  Detection methods:
 *  1. Process scanner — catches known stealers by name
 *  2. Suspicious process detector — flags .exe from Temp/Downloads
 *  3. Honeypot canary files — fake session files that stealers grab
 *  4. File monitor — catches file creation/deletion/renaming (not reads)
 *
 *  Build:  cmake -B build && cmake --build build --config Release
 *  Run:    WenzGuard.exe [--silent] [--scan-only] [--test]
 */

#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <algorithm>
#include <map>
#include <chrono>

#include "browser_paths.h"
#include "monitor.h"
#include "alert.h"
#include "logger.h"
#include "process_scanner.h"
#include "honeypot.h"

static std::atomic<bool> g_running{true};
static SessionMonitor* g_monitor = nullptr;

BOOL WINAPI console_handler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        g_running.store(false);
        if (g_monitor) g_monitor->stop();
        return TRUE;
    }
    return FALSE;
}

void print_banner() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    printf("\n");
    printf("  ======================================\n");
    printf("   W E N Z   G U A R D  v3.0\n");
    printf("   Session Security Monitor\n");
    printf("  ======================================\n\n");
}

void run_process_scan(bool verbose = false) {
    auto suspicious = scan_suspicious_processes();

    if (suspicious.empty()) {
        if (verbose) printf("  [OK] No suspicious processes found\n");
        return;
    }

    for (const auto& proc : suspicious) {
        printf("  [!] %ls (PID %lu) - %ls\n",
               proc.name.c_str(), proc.pid, proc.reason.c_str());
        if (!proc.path.empty())
            printf("      Path: %ls\n", proc.path.c_str());

        std::wstring lower = proc.name;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);

        if (KNOWN_STEALERS.count(lower) > 0) {
            play_alarm_sound();
            show_notification(
                L"STEALER: " + proc.name,
                L"PID " + std::to_wstring(proc.pid) + L"\n" + proc.path
            );
        } else {
            show_notification(
                L"Suspicious: " + proc.name,
                proc.reason + L"\n" + proc.path
            );
        }
    }
}

void periodic_scan_thread() {
    while (g_running.load()) {
        for (int i = 0; i < 30 && g_running.load(); i++) Sleep(1000);
        if (!g_running.load()) break;
        run_process_scan();
    }
}

// Extract just the filename from full path
std::wstring short_filename(const std::wstring& full_path) {
    auto pos = full_path.find_last_of(L"\\");
    if (pos != std::wstring::npos) return full_path.substr(pos + 1);
    return full_path;
}

// Check if a file change is from a honeypot canary
bool is_honeypot_file(const std::wstring& path, const std::vector<HoneypotFile>& honeypots,
                      std::wstring& app_out, std::wstring& dtype_out) {
    for (const auto& hp : honeypots) {
        if (path.find(short_filename(hp.path)) != std::wstring::npos) {
            app_out = hp.app_name;
            dtype_out = hp.data_type;
            return true;
        }
    }
    return false;
}

// Detect what type of data was stolen
std::wstring detect_stolen_type(const std::wstring& path) {
    std::wstring p = path;
    std::transform(p.begin(), p.end(), p.begin(), ::towlower);

    if (p.find(L"local state") != std::wstring::npos)         return L"MASTER KEY";
    if (p.find(L"login data") != std::wstring::npos)           return L"PASSWORDS";
    if (p.find(L"cookies") != std::wstring::npos)              return L"COOKIES";
    if (p.find(L"web data") != std::wstring::npos)             return L"AUTOFILL/CARDS";
    if (p.find(L"key_data") != std::wstring::npos)             return L"TELEGRAM SESSION";
    if (p.find(L"d877f783d5d3ef8c") != std::wstring::npos)     return L"TELEGRAM SESSION";
    if (p.find(L".ldb") != std::wstring::npos &&
        p.find(L"discord") != std::wstring::npos)              return L"DISCORD TOKEN";
    if (p.find(L"ssfn") != std::wstring::npos)                 return L"STEAM GUARD";
    if (p.find(L"loginusers.vdf") != std::wstring::npos)       return L"STEAM ACCOUNT";
    if (p.find(L"seed") != std::wstring::npos)                 return L"CRYPTO SEED";
    if (p.find(L"wallet") != std::wstring::npos)               return L"CRYPTO WALLET";
    if (p.find(L".git-credentials") != std::wstring::npos)     return L"GIT CREDENTIALS";
    if (p.find(L"key4.db") != std::wstring::npos)              return L"FIREFOX KEYS";
    if (p.find(L"logins.json") != std::wstring::npos)          return L"FIREFOX PASSWORDS";
    return L"SESSION DATA";
}

// Test mode: simulate a stealer touching honeypot files
void run_test(const std::vector<HoneypotFile>& honeypots) {
    printf("\n  === TEST MODE ===\n\n");

    if (honeypots.empty()) {
        printf("  No honeypot files found. Cannot test.\n");
        return;
    }

    printf("  Simulating stealer in 3 seconds...\n");
    printf("  You should see notifications for each honeypot.\n\n");
    Sleep(3000);

    for (const auto& hp : honeypots) {
        printf("  [TEST] Touching: %ls (%ls)\n",
               hp.app_name.c_str(), hp.data_type.c_str());

        // Touch the file (update timestamp = triggers FILE_NOTIFY_CHANGE_LAST_WRITE)
        HANDLE hFile = CreateFileW(
            hp.path.c_str(), FILE_WRITE_ATTRIBUTES,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_HIDDEN, NULL
        );
        if (hFile != INVALID_HANDLE_VALUE) {
            FILETIME ft;
            GetSystemTimeAsFileTime(&ft);
            SetFileTime(hFile, NULL, NULL, &ft);
            CloseHandle(hFile);
        }

        Sleep(500);
    }

    printf("\n  [TEST] Done! Check your notifications.\n");
    printf("  Press Ctrl+C to stop.\n\n");
}

int wmain(int argc, wchar_t* argv[]) {
    bool silent = false;
    bool scan_only = false;
    bool test_mode = false;

    for (int i = 1; i < argc; i++) {
        std::wstring arg = argv[i];
        if (arg == L"--silent" || arg == L"-s") silent = true;
        if (arg == L"--scan-only" || arg == L"--scan") scan_only = true;
        if (arg == L"--test" || arg == L"-t") test_mode = true;
        if (arg == L"--help" || arg == L"-h") {
            printf("WenzGuard v3.0 - Session Security Monitor\n\n");
            printf("  --silent, -s     No notifications (log only)\n");
            printf("  --scan-only      Scan processes and exit\n");
            printf("  --test, -t       Test mode: touch honeypots to verify alerts\n");
            printf("  --help, -h       This help\n");
            return 0;
        }
    }

    print_banner();
    SetConsoleCtrlHandler(console_handler, TRUE);

    // Logger
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(NULL, exe_path, MAX_PATH);
    std::wstring log_path = exe_path;
    log_path = log_path.substr(0, log_path.find_last_of(L'\\') + 1) + L"wenzguard.log";
    Logger::instance().init(log_path);

    // === DISCOVER ===
    auto browsers = discover_browser_profiles();
    auto apps = discover_app_profiles();
    int total = (int)browsers.size() + (int)apps.size();

    printf("  Found: %d browsers, %d apps\n\n", (int)browsers.size(), (int)apps.size());

    for (const auto& b : browsers)
        printf("    [WEB]  %ls\n", b.browser_name.c_str());

    std::map<std::wstring, std::vector<const AppProfile*>> by_cat;
    for (const auto& a : apps) by_cat[a.category].push_back(&a);

    struct { const wchar_t* id; const char* label; } cats[] = {
        { L"messenger", "MSG" }, { L"gaming", "GAME" },
        { L"crypto", "CRYPTO" }, { L"vpn", "VPN" }, { L"other", "APP" },
    };
    for (auto& [id, label] : cats) {
        auto it = by_cat.find(id);
        if (it == by_cat.end()) continue;
        for (const auto* a : it->second)
            printf("    [%s] %ls\n", label, a->app_name.c_str());
    }

    // === HONEYPOTS ===
    printf("\n");
    auto honeypots = deploy_honeypots();

    // === PROCESS SCAN ===
    printf("\n  Scanning processes...\n");
    run_process_scan(true);

    if (scan_only) {
        cleanup_honeypots(honeypots);
        printf("\n  Scan complete.\n");
        return 0;
    }

    // === MONITORING ===
    SessionMonitor monitor;
    g_monitor = &monitor;

    for (const auto& b : browsers)
        monitor.add_watch(b.profile_path, b.browser_name);
    for (const auto& a : apps)
        monitor.add_watch(a.profile_path, a.app_name);

    // Throttle
    static auto last_alert_time = std::chrono::steady_clock::now();

    monitor.set_alert_callback([&](const FileEvent& event) {
        std::wstring file = short_filename(event.file_path);

        // === HONEYPOT CHECK (highest priority) ===
        std::wstring hp_app, hp_dtype;
        if (is_honeypot_file(event.file_path, honeypots, hp_app, hp_dtype)) {
            // ANY access to honeypot = stealer!
            play_alarm_sound();
            play_alarm_sound();

            printf("\n  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            printf("  !! HONEYPOT TRIGGERED !!          !!\n");
            printf("  !! %ls - %ls\n",
                   hp_app.c_str(), hp_dtype.c_str());
            printf("  !! File: %ls\n", file.c_str());
            printf("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");

            Logger::instance().alert(
                L"HONEYPOT TRIGGERED: " + hp_app + L" " + hp_dtype +
                L" | " + event.file_path);

            if (!silent) {
                show_notification(
                    L"STEALER DETECTED! " + hp_dtype,
                    hp_app + L" canary was touched!\n" + file
                );
            }

            // Immediately scan processes to find the culprit
            printf("  Scanning for culprit...\n");
            run_process_scan(true);
            return;
        }

        // === NORMAL FILE MONITORING ===
        // Ignore MODIFIED — that's usually the legitimate app itself
        if (event.action == L"MODIFIED") return;

        // Only alert on CREATED / DELETED / RENAMED (suspicious)
        std::wstring stolen = detect_stolen_type(event.file_path);

        Logger::instance().info(
            L"[" + event.browser_name + L"] " + stolen + L" " +
            event.action + L": " + file);

        if (silent) return;

        // Throttle
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - last_alert_time).count();
        if (elapsed < 3) return;
        last_alert_time = now;

        play_alarm_sound();
        show_notification(
            stolen + L" — " + event.browser_name,
            file + L" was " + event.action
        );
    });

    // Periodic process scanning
    std::thread scan_thread(periodic_scan_thread);
    scan_thread.detach();

    printf("\n  ======================================\n");
    printf("  Monitoring active. Ctrl+C to stop.\n");
    printf("  ======================================\n\n");

    // Test mode
    if (test_mode) {
        std::thread test_thread([&honeypots]() {
            run_test(honeypots);
        });
        test_thread.detach();
    }

    monitor.start();

    // Cleanup
    printf("\n  Cleaning up honeypots...\n");
    cleanup_honeypots(honeypots);
    g_monitor = nullptr;
    printf("  WenzGuard stopped.\n");
    return 0;
}
