/*
 *  WenzGuard — Монитор безопасности сессий
 *
 *  Следит за сессиями браузеров, мессенджеров, игровых платформ,
 *  крипто-кошельков и других программ. При подозрительном обращении
 *  к файлам сессий — уведомляет пользователя.
 *
 *  Сборка: cmake -B build && cmake --build build --config Release
 *  Запуск: WenzGuard.exe [--silent] [--scan-only]
 */

#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <algorithm>
#include <map>

#include "browser_paths.h"
#include "monitor.h"
#include "alert.h"
#include "logger.h"
#include "process_scanner.h"

static std::atomic<bool> g_running{true};
static SessionMonitor* g_monitor = nullptr;

BOOL WINAPI console_handler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        std::wcout << L"\n[WenzGuard] Остановка..." << std::endl;
        g_running.store(false);
        if (g_monitor) g_monitor->stop();
        return TRUE;
    }
    return FALSE;
}

void print_banner() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::wcout << L"\n";
    std::wcout << L"  ╦ ╦╔═╗╔╗╔╔═╗  ╔═╗╦ ╦╔═╗╦═╗╔╦╗\n";
    std::wcout << L"  ║║║║╣ ║║║╔═╝  ║ ╦║ ║╠═╣╠╦╝ ║║\n";
    std::wcout << L"  ╚╩╝╚═╝╝╚╝╚═╝  ╚═╝╚═╝╩ ╩╩╚══╩╝\n";
    std::wcout << L"\n";
    std::wcout << L"  Монитор безопасности сессий v2.0\n";
    std::wcout << L"  ─────────────────────────────────\n\n";
}

void run_process_scan() {
    Logger::instance().info(L"Сканирование процессов...");
    auto suspicious = scan_suspicious_processes();

    if (suspicious.empty()) {
        Logger::instance().info(L"Подозрительных процессов не обнаружено ✓");
    } else {
        for (const auto& proc : suspicious) {
            std::wstring msg = L"⚠ ПОДОЗРИТЕЛЬНЫЙ: " + proc.name +
                               L" (PID: " + std::to_wstring(proc.pid) + L") — " + proc.reason;
            if (!proc.path.empty()) msg += L" | " + proc.path;
            Logger::instance().alert(msg);

            std::wstring lower = proc.name;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
            if (KNOWN_STEALERS.count(lower) > 0) {
                play_alarm_sound();
                show_alert(
                    L"⚠ WenzGuard — ОБНАРУЖЕН СТИЛЕР!",
                    L"Обнаружен: " + proc.name +
                    L"\nPID: " + std::to_wstring(proc.pid) +
                    L"\nПуть: " + proc.path +
                    L"\n\nНЕМЕДЛЕННО завершите этот процесс!"
                );
            }
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

// Категория -> эмодзи для вывода
std::wstring category_icon(const std::wstring& cat) {
    if (cat == L"messenger") return L"💬";
    if (cat == L"gaming")    return L"🎮";
    if (cat == L"crypto")    return L"💰";
    if (cat == L"vpn")       return L"🔒";
    return L"📁";
}

int wmain(int argc, wchar_t* argv[]) {
    bool silent = false;
    bool scan_only = false;

    for (int i = 1; i < argc; i++) {
        std::wstring arg = argv[i];
        if (arg == L"--silent" || arg == L"-s") silent = true;
        if (arg == L"--scan-only" || arg == L"--scan") scan_only = true;
        if (arg == L"--help" || arg == L"-h") {
            std::wcout << L"WenzGuard — Монитор безопасности сессий v2.0\n\n";
            std::wcout << L"  --silent, -s     Без всплывающих окон\n";
            std::wcout << L"  --scan-only      Только сканирование процессов\n";
            std::wcout << L"  --help, -h       Справка\n";
            return 0;
        }
    }

    print_banner();
    SetConsoleCtrlHandler(console_handler, TRUE);

    // Логгер
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(NULL, exe_path, MAX_PATH);
    std::wstring log_path = exe_path;
    log_path = log_path.substr(0, log_path.find_last_of(L'\\') + 1) + L"wenzguard.log";
    Logger::instance().init(log_path);
    Logger::instance().info(L"WenzGuard v2.0 запущен");

    // ===== Обнаружение =====

    // 1. Браузеры
    auto browsers = discover_browser_profiles();
    // 2. Программы и мессенджеры
    auto apps = discover_app_profiles();

    int total = (int)browsers.size() + (int)apps.size();

    if (total == 0) {
        Logger::instance().warn(L"Ничего не найдено для мониторинга!");
        return 1;
    }

    // Вывод найденного
    std::wcout << L"  ── Браузеры (" << browsers.size() << L") ──\n";
    for (const auto& b : browsers) {
        std::wcout << L"  🌐 " << b.browser_name << L"\n";
    }

    // Группируем приложения по категориям
    std::map<std::wstring, std::vector<const AppProfile*>> by_category;
    for (const auto& a : apps) {
        by_category[a.category].push_back(&a);
    }

    const std::vector<std::pair<std::wstring, std::wstring>> category_names = {
        { L"messenger", L"Мессенджеры" },
        { L"gaming",    L"Игровые платформы" },
        { L"crypto",    L"Крипто-кошельки" },
        { L"vpn",       L"VPN" },
        { L"other",     L"Другие" },
    };

    for (const auto& [cat_id, cat_name] : category_names) {
        auto it = by_category.find(cat_id);
        if (it == by_category.end()) continue;

        std::wcout << L"\n  ── " << cat_name << L" (" << it->second.size() << L") ──\n";
        for (const auto* a : it->second) {
            std::wcout << L"  " << category_icon(cat_id) << L" " << a->app_name << L"\n";
        }
    }

    std::wcout << L"\n  Всего: " << total << L" профилей\n\n";

    // Сканирование процессов
    run_process_scan();

    if (scan_only) {
        std::wcout << L"\n  Сканирование завершено.\n";
        return 0;
    }

    // ===== Мониторинг =====

    SessionMonitor monitor;
    g_monitor = &monitor;

    for (const auto& b : browsers) {
        monitor.add_watch(b.profile_path, L"🌐 " + b.browser_name);
    }
    for (const auto& a : apps) {
        monitor.add_watch(a.profile_path, category_icon(a.category) + L" " + a.app_name);
    }

    // Подавление спама
    static auto last_alert_time = std::chrono::steady_clock::now();

    monitor.set_alert_callback([&](const FileEvent& event) {
        std::wstring lower_path = event.file_path;
        std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(), ::towlower);

        // Определяем критичность
        bool is_critical =
            // Браузеры
            lower_path.find(L"cookies") != std::wstring::npos ||
            lower_path.find(L"login data") != std::wstring::npos ||
            lower_path.find(L"local state") != std::wstring::npos ||
            lower_path.find(L"key4.db") != std::wstring::npos ||
            lower_path.find(L"logins.json") != std::wstring::npos ||
            // Telegram
            lower_path.find(L"key_data") != std::wstring::npos ||
            lower_path.find(L"d877f783d5d3ef8c") != std::wstring::npos ||
            // Discord
            (lower_path.find(L"discord") != std::wstring::npos && lower_path.find(L".ldb") != std::wstring::npos) ||
            // Steam
            lower_path.find(L"ssfn") != std::wstring::npos ||
            lower_path.find(L"loginusers.vdf") != std::wstring::npos ||
            // Крипто
            lower_path.find(L"seed.seco") != std::wstring::npos ||
            lower_path.find(L"default_wallet") != std::wstring::npos ||
            // Git
            lower_path.find(L".git-credentials") != std::wstring::npos;

        bool is_session =
            lower_path.find(L"session") != std::wstring::npos ||
            lower_path.find(L"tabs") != std::wstring::npos ||
            lower_path.find(L"tdata") != std::wstring::npos ||
            lower_path.find(L"map0") != std::wstring::npos ||
            lower_path.find(L"config.vdf") != std::wstring::npos;

        std::wstring log_msg = L"[" + event.browser_name + L"] " +
                               event.action + L": " + event.file_path;

        if (is_critical) {
            Logger::instance().alert(L"🔴 КРИТИЧНО " + log_msg);
        } else if (is_session) {
            Logger::instance().warn(L"🟡 СЕССИЯ " + log_msg);
        } else {
            Logger::instance().info(L"🔵 " + log_msg);
        }

        // Алерт (не чаще раз в 5 сек)
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_alert_time).count();

        if (is_critical && elapsed >= 5 && !silent) {
            last_alert_time = now;
            play_alarm_sound();

            std::wstring alert_msg =
                L"Подозрительное обращение к данным!\n\n"
                L"Приложение: " + event.browser_name + L"\n"
                L"Файл: " + event.file_path + L"\n"
                L"Действие: " + event.action + L"\n"
                L"Время: " + event.timestamp + L"\n\n"
                L"Если вы не открывали это приложение — возможно,\n"
                L"стилер пытается украсть ваши данные!\n\n"
                L"1. Откройте Диспетчер задач (Ctrl+Shift+Esc)\n"
                L"2. Ищите подозрительные процессы\n"
                L"3. Проверьте ПК антивирусом";

            std::thread([alert_msg]() {
                show_alert(L"⚠ WenzGuard — ТРЕВОГА!", alert_msg);
            }).detach();
        }
    });

    // Периодическое сканирование процессов
    std::thread scan_thread(periodic_scan_thread);
    scan_thread.detach();

    std::wcout << L"  ─────────────────────────────────\n";
    std::wcout << L"  Мониторинг запущен. Ctrl+C для выхода.\n";
    std::wcout << L"  ─────────────────────────────────\n\n";

    Logger::instance().info(L"Мониторинг: " + std::to_wstring(total) + L" профилей");

    monitor.start();

    g_monitor = nullptr;
    Logger::instance().info(L"WenzGuard остановлен.");
    return 0;
}
