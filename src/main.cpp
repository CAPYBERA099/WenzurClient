/*
 *  WenzGuard v3.2 — Session Security Monitor
 *
 *  Detection methods:
 *  1. Real-time process tracker — detects NEW suspicious processes every 2 sec
 *  2. Known stealer database — 50+ stealer names
 *  3. Location detector — flags .exe from Temp/Downloads/Desktop
 *  4. Honeypot canary files — fake session files stealers try to grab
 *  5. File monitor — catches file creation/deletion in session dirs
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
#include <set>
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
    printf("   W E N Z   G U A R D  v3.2\n");
    printf("   Session Security Monitor\n");
    printf("  ======================================\n\n");
}

// ============ REAL-TIME PROCESS TRACKER ============
// Scans every 2 seconds, tracks PIDs, alerts on NEW suspicious processes

void realtime_process_tracker() {
    std::set<DWORD> known_pids;
    bool first_scan = true;

    while (g_running.load()) {
        auto all = get_all_processes();

        for (const auto& proc : all) {
            // Skip already-seen PIDs
            if (known_pids.count(proc.pid) > 0) continue;
            known_pids.insert(proc.pid);

            // On first scan, just record existing PIDs without alerting
            if (first_scan) continue;

            std::wstring lower_name = proc.name;
            std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::towlower);

            // Skip known safe processes
            if (is_browser_process(proc.name)) continue;
            if (SYSTEM_PROCESSES.count(lower_name) > 0) continue;

            // CHECK 1: Known stealer
            if (KNOWN_STEALERS.count(lower_name) > 0) {
                play_alarm_sound();
                play_alarm_sound();
                play_alarm_sound();

                std::wstring title = L"!! STEALER: " + proc.name + L" !!";
                std::wstring body = L"PID: " + std::to_wstring(proc.pid);
                if (!proc.path.empty()) body += L"\n" + proc.path;

                show_notification(title, body);
                Logger::instance().alert(title + L" | " + body);
                continue;
            }

            // CHECK 2: Suspicious process name
            if (is_suspicious_process(proc.name)) {
                play_alarm_sound();

                std::wstring title = L"Suspicious: " + proc.name;
                std::wstring body = L"PID: " + std::to_wstring(proc.pid);
                if (!proc.path.empty()) body += L"\n" + proc.path;

                show_notification(title, body);
                Logger::instance().warn(title + L" | " + body);
                continue;
            }

            // CHECK 3: Running from suspicious location
            if (!proc.path.empty() && is_suspicious_location(proc.path)
                && !is_safe_location(proc.path)) {
                play_alarm_sound();

                std::wstring title = L"New process: " + proc.name;
                std::wstring body = proc.path;

                show_notification(title, body);
                Logger::instance().warn(L"FROM TEMP/DOWNLOADS: " + proc.name + L" | " + proc.path);
            }
        }

        first_scan = false;

        // Clean up stale PIDs periodically (every ~30 seconds = 15 iterations)
        static int cleanup_counter = 0;
        if (++cleanup_counter >= 15) {
            cleanup_counter = 0;
            std::set<DWORD> current_pids;
            for (const auto& p : all) current_pids.insert(p.pid);

            std::set<DWORD> to_remove;
            for (DWORD pid : known_pids) {
                if (current_pids.count(pid) == 0) to_remove.insert(pid);
            }
            for (DWORD pid : to_remove) known_pids.erase(pid);
        }

        // Sleep 2 seconds (in small chunks for responsiveness)
        for (int i = 0; i < 20 && g_running.load(); i++) Sleep(100);
    }
}

// ============ FILE EVENT HELPERS ============

std::wstring short_filename(const std::wstring& full_path) {
    auto pos = full_path.find_last_of(L"\\");
    if (pos != std::wstring::npos) return full_path.substr(pos + 1);
    return full_path;
}

bool is_honeypot_file(const std::wstring& path, const std::vector<HoneypotFile>& honeypots,
                      std::wstring& app_out, std::wstring& dtype_out) {
    std::wstring lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);

    for (const auto& hp : honeypots) {
        std::wstring hp_name = short_filename(hp.path);
        std::transform(hp_name.begin(), hp_name.end(), hp_name.begin(), ::towlower);

        if (lower.find(hp_name) != std::wstring::npos) {
            app_out = hp.app_name;
            dtype_out = hp.data_type;
            return true;
        }
    }
    return false;
}

std::wstring detect_stolen_type(const std::wstring& path) {
    std::wstring p = path;
    std::transform(p.begin(), p.end(), p.begin(), ::towlower);

    // Browser credentials
    if (p.find(L"local state") != std::wstring::npos)         return L"MASTER KEY";
    if (p.find(L"login data") != std::wstring::npos)           return L"PASSWORDS";
    if (p.find(L"web data") != std::wstring::npos)             return L"AUTOFILL/CARDS";
    if (p.find(L"key4.db") != std::wstring::npos)              return L"FIREFOX KEYS";
    if (p.find(L"logins.json") != std::wstring::npos)          return L"FIREFOX PASSWORDS";

    // Cookies — check path context
    if (p.find(L"cookies") != std::wstring::npos) {
        if (p.find(L"\\chrome\\") != std::wstring::npos)       return L"CHROME COOKIES";
        if (p.find(L"\\edge\\") != std::wstring::npos)         return L"EDGE COOKIES";
        if (p.find(L"\\opera") != std::wstring::npos)          return L"OPERA COOKIES";
        if (p.find(L"\\brave") != std::wstring::npos)          return L"BRAVE COOKIES";
        if (p.find(L"\\yandex") != std::wstring::npos)         return L"YANDEX COOKIES";
        if (p.find(L"\\firefox\\") != std::wstring::npos)      return L"FIREFOX COOKIES";
        return L"COOKIES";
    }

    // Telegram
    if (p.find(L"key_data") != std::wstring::npos)             return L"TELEGRAM SESSION";
    if (p.find(L"d877f783d5d3ef8c") != std::wstring::npos)     return L"TELEGRAM SESSION";

    // Discord — only if path actually has "discord" in it
    if (p.find(L"\\discord") != std::wstring::npos)            return L"DISCORD TOKEN";

    // Steam
    if (p.find(L"ssfn") != std::wstring::npos)                 return L"STEAM GUARD";
    if (p.find(L"loginusers.vdf") != std::wstring::npos)       return L"STEAM ACCOUNT";

    // Crypto
    if (p.find(L"seed") != std::wstring::npos)                 return L"CRYPTO SEED";
    if (p.find(L"wallet") != std::wstring::npos)               return L"CRYPTO WALLET";

    // Git
    if (p.find(L".git-credentials") != std::wstring::npos)     return L"GIT CREDENTIALS";

    // Honeypot
    if (p.find(L"canary") != std::wstring::npos ||
        p.find(L".wgcanary") != std::wstring::npos)            return L"HONEYPOT";

    return L"SESSION DATA";
}

// ============ TEST MODE ============

void run_test(const std::vector<HoneypotFile>& honeypots) {
    printf("\n  === TEST MODE ===\n\n");

    if (honeypots.empty()) {
        printf("  No honeypot files found. Cannot test.\n");
        return;
    }

    printf("  Simulating stealer in 3 seconds...\n");
    printf("  You should see notifications for each canary.\n\n");
    Sleep(3000);

    for (const auto& hp : honeypots) {
        printf("  [TEST] Touching: %ls (%ls)\n",
               hp.app_name.c_str(), hp.data_type.c_str());

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
        } else {
            printf("    (could not touch: %lu)\n", GetLastError());
        }

        Sleep(1000);
    }

    printf("\n  [TEST] Done! Check your notifications.\n");
    printf("  Press Ctrl+C to stop.\n\n");
}

// ============ MAIN ============

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
            printf("WenzGuard v3.2 - Session Security Monitor\n\n");
            printf("  --silent, -s     No notifications (log only)\n");
            printf("  --scan-only      Scan processes and exit\n");
            printf("  --test, -t       Test: touch honeypots to verify alerts\n");
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

    // === DISCOVER PROFILES ===
    auto browsers = discover_browser_profiles();
    auto apps = discover_app_profiles();

    printf("  Browsers: %d | Apps: %d\n", (int)browsers.size(), (int)apps.size());

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

    // === DEPLOY HONEYPOTS ===
    printf("\n");
    auto honeypots = deploy_honeypots();

    // === INITIAL PROCESS SCAN ===
    printf("\n  Initial process scan...\n");
    auto suspicious = scan_suspicious_processes();
    if (suspicious.empty()) {
        printf("  [OK] No suspicious processes\n");
    } else {
        for (const auto& s : suspicious) {
            printf("  [!] %ls (PID %lu) - %ls\n",
                   s.name.c_str(), s.pid, s.reason.c_str());
            if (!s.path.empty())
                printf("      %ls\n", s.path.c_str());
        }
    }

    if (scan_only) {
        cleanup_honeypots(honeypots);
        printf("\n  Done.\n");
        return 0;
    }

    // === FILE MONITOR ===
    SessionMonitor monitor;
    g_monitor = &monitor;

    for (const auto& b : browsers)
        monitor.add_watch(b.profile_path, b.browser_name);
    for (const auto& a : apps)
        monitor.add_watch(a.profile_path, a.app_name);

    // Throttle for file events
    static auto last_file_alert = std::chrono::steady_clock::now();

    monitor.set_alert_callback([&](const FileEvent& event) {
        std::wstring file = short_filename(event.file_path);

        // === HONEYPOT ===
        std::wstring hp_app, hp_dtype;
        if (is_honeypot_file(event.file_path, honeypots, hp_app, hp_dtype)) {
            play_alarm_sound();
            play_alarm_sound();

            std::wstring title = L"!! " + hp_dtype + L" STOLEN !!";
            std::wstring body = hp_app + L" canary triggered\n" + event.file_path;

            show_notification(title, body);
            Logger::instance().alert(L"HONEYPOT: " + hp_app + L" " + hp_dtype + L" | " + event.file_path);

            // Find the culprit
            auto culprits = scan_suspicious_processes();
            for (const auto& c : culprits) {
                show_notification(
                    L"STEALER PROCESS: " + c.name,
                    L"PID " + std::to_wstring(c.pid) + L"\n" + c.path
                );
            }
            return;
        }

        // === SKIP MODIFIED ===
        if (event.action == L"MODIFIED") return;

        // Log all non-modified events
        std::wstring stolen = detect_stolen_type(event.file_path);
        Logger::instance().info(
            L"[" + event.browser_name + L"] " + stolen + L" " +
            event.action + L": " + event.file_path);

        if (silent) return;

        // Throttle file alerts (10 sec per source to avoid spam)
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - last_file_alert).count();
        if (elapsed < 10) return;
        last_file_alert = now;

        play_alarm_sound();
        show_notification(
            stolen + L" - " + event.browser_name,
            event.action + L": " + event.file_path
        );
    });

    // === START REAL-TIME PROCESS TRACKER ===
    printf("\n  Starting real-time process tracker (every 2s)...\n");
    std::thread tracker_thread(realtime_process_tracker);
    tracker_thread.detach();

    printf("\n  ======================================\n");
    printf("  ACTIVE. Ctrl+C to stop.\n");
    printf("  ======================================\n\n");

    // Test mode
    if (test_mode) {
        std::thread test_thread([&honeypots]() {
            run_test(honeypots);
        });
        test_thread.detach();
    }

    // Start file monitoring (blocks)
    monitor.start();

    // Cleanup
    printf("\n  Cleaning up...\n");
    cleanup_honeypots(honeypots);
    cleanup_tray();
    g_monitor = nullptr;
    printf("  WenzGuard stopped.\n");
    return 0;
}
