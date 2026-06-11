/*
 *  WenzGuard v2.1 — Session Security Monitor
 *  Monitors browser, messenger, gaming and crypto sessions.
 *  Alerts via Windows toast notifications.
 *
 *  Build:  cmake -B build && cmake --build build --config Release
 *  Run:    WenzGuard.exe [--silent] [--scan-only]
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
    printf("  =============================\n");
    printf("   W E N Z   G U A R D  v2.1\n");
    printf("  =============================\n");
    printf("  Session Security Monitor\n\n");
}

void run_process_scan() {
    auto suspicious = scan_suspicious_processes();

    if (suspicious.empty()) return;

    for (const auto& proc : suspicious) {
        std::wstring lower = proc.name;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);

        if (KNOWN_STEALERS.count(lower) > 0) {
            play_alarm_sound();
            show_notification(
                L"STEALER FOUND: " + proc.name,
                L"PID " + std::to_wstring(proc.pid) + L"\n" + proc.path
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

// What was stolen — short label
std::wstring detect_stolen_type(const std::wstring& path) {
    std::wstring p = path;
    std::transform(p.begin(), p.end(), p.begin(), ::towlower);

    if (p.find(L"local state") != std::wstring::npos)         return L"MASTER KEY";
    if (p.find(L"login data") != std::wstring::npos)           return L"PASSWORDS";
    if (p.find(L"cookies") != std::wstring::npos)              return L"COOKIES";
    if (p.find(L"key_data") != std::wstring::npos)             return L"TELEGRAM SESSION";
    if (p.find(L"d877f783d5d3ef8c") != std::wstring::npos)     return L"TELEGRAM SESSION";
    if (p.find(L"a7fdf864fbc10b77") != std::wstring::npos)     return L"TELEGRAM SESSION";
    if (p.find(L"discord") != std::wstring::npos && p.find(L".ldb") != std::wstring::npos)
                                                                return L"DISCORD TOKEN";
    if (p.find(L"ssfn") != std::wstring::npos)                 return L"STEAM GUARD";
    if (p.find(L"loginusers.vdf") != std::wstring::npos)       return L"STEAM ACCOUNT";
    if (p.find(L"config.vdf") != std::wstring::npos)           return L"STEAM CONFIG";
    if (p.find(L"seed") != std::wstring::npos)                 return L"CRYPTO SEED";
    if (p.find(L"wallet") != std::wstring::npos)               return L"CRYPTO WALLET";
    if (p.find(L".git-credentials") != std::wstring::npos)     return L"GIT CREDENTIALS";
    if (p.find(L"key4.db") != std::wstring::npos)              return L"FIREFOX KEYS";
    if (p.find(L"logins.json") != std::wstring::npos)          return L"FIREFOX PASSWORDS";
    if (p.find(L"web data") != std::wstring::npos)             return L"AUTOFILL / CARDS";
    if (p.find(L"session") != std::wstring::npos)              return L"SESSION";
    if (p.find(L"tabs") != std::wstring::npos)                 return L"TABS";
    if (p.find(L"history") != std::wstring::npos)              return L"HISTORY";
    return L"SESSION DATA";
}

// Is this a critical file worth alerting about?
bool is_critical_file(const std::wstring& path) {
    std::wstring p = path;
    std::transform(p.begin(), p.end(), p.begin(), ::towlower);

    return p.find(L"cookies") != std::wstring::npos
        || p.find(L"login data") != std::wstring::npos
        || p.find(L"local state") != std::wstring::npos
        || p.find(L"key4.db") != std::wstring::npos
        || p.find(L"logins.json") != std::wstring::npos
        || p.find(L"key_data") != std::wstring::npos
        || p.find(L"d877f783d5d3ef8c") != std::wstring::npos
        || (p.find(L"discord") != std::wstring::npos && p.find(L".ldb") != std::wstring::npos)
        || p.find(L"ssfn") != std::wstring::npos
        || p.find(L"loginusers.vdf") != std::wstring::npos
        || p.find(L"seed") != std::wstring::npos
        || p.find(L"wallet") != std::wstring::npos
        || p.find(L".git-credentials") != std::wstring::npos
        || p.find(L"web data") != std::wstring::npos;
}

// Extract just the filename from full path
std::wstring short_filename(const std::wstring& full_path) {
    auto pos = full_path.find_last_of(L"\\");
    if (pos != std::wstring::npos)
        return full_path.substr(pos + 1);
    return full_path;
}

int wmain(int argc, wchar_t* argv[]) {
    bool silent = false;
    bool scan_only = false;

    for (int i = 1; i < argc; i++) {
        std::wstring arg = argv[i];
        if (arg == L"--silent" || arg == L"-s") silent = true;
        if (arg == L"--scan-only" || arg == L"--scan") scan_only = true;
        if (arg == L"--help" || arg == L"-h") {
            printf("WenzGuard v2.1 - Session Security Monitor\n\n");
            printf("  --silent, -s     No notifications (log only)\n");
            printf("  --scan-only      Scan processes and exit\n");
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

    // Discover profiles
    auto browsers = discover_browser_profiles();
    auto apps = discover_app_profiles();
    int total = (int)browsers.size() + (int)apps.size();

    if (total == 0) {
        printf("  Nothing found to monitor.\n");
        return 1;
    }

    // Print discovered
    printf("  Browsers: %d\n", (int)browsers.size());
    for (const auto& b : browsers) {
        printf("    [WEB] %ls\n", b.browser_name.c_str());
    }

    // Group apps by category
    std::map<std::wstring, std::vector<const AppProfile*>> by_cat;
    for (const auto& a : apps) by_cat[a.category].push_back(&a);

    struct { const wchar_t* id; const char* name; } cats[] = {
        { L"messenger", "Messengers" },
        { L"gaming",    "Gaming" },
        { L"crypto",    "Crypto" },
        { L"vpn",       "VPN" },
        { L"other",     "Other" },
    };

    for (auto& [id, name] : cats) {
        auto it = by_cat.find(id);
        if (it == by_cat.end()) continue;
        printf("\n  %s: %d\n", name, (int)it->second.size());
        for (const auto* a : it->second) {
            printf("    [+] %ls\n", a->app_name.c_str());
        }
    }

    printf("\n  Total: %d profiles\n\n", total);

    // Process scan
    run_process_scan();

    if (scan_only) {
        printf("  Scan complete.\n");
        return 0;
    }

    // Setup monitoring
    SessionMonitor monitor;
    g_monitor = &monitor;

    for (const auto& b : browsers) {
        monitor.add_watch(b.profile_path, b.browser_name);
    }
    for (const auto& a : apps) {
        monitor.add_watch(a.profile_path, a.app_name);
    }

    // Throttle
    static auto last_alert_time = std::chrono::steady_clock::now();

    monitor.set_alert_callback([&](const FileEvent& event) {
        // Log everything
        Logger::instance().info(L"[" + event.browser_name + L"] " + event.action + L": " + event.file_path);

        // Only alert on critical files
        if (!is_critical_file(event.file_path)) return;
        if (silent) return;

        // Throttle: max 1 notification per 3 seconds
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_alert_time).count();
        if (elapsed < 3) return;
        last_alert_time = now;

        play_alarm_sound();

        // Clean notification:
        // Title: "COOKIES STOLEN — Chrome"
        // Body:  "Cookies-journal was MODIFIED"
        std::wstring stolen = detect_stolen_type(event.file_path);
        std::wstring file = short_filename(event.file_path);

        std::wstring title = stolen + L" — " + event.browser_name;
        std::wstring body = file + L" was " + event.action;

        show_notification(title, body);
    });

    // Periodic process scan in background
    std::thread scan_thread(periodic_scan_thread);
    scan_thread.detach();

    printf("  Monitoring active. Ctrl+C to stop.\n");
    printf("  ====================================\n\n");

    monitor.start();

    g_monitor = nullptr;
    return 0;
}
