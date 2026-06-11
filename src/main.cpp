/*
 *  WenzGuard — Монитор безопасности сессий и cookies браузеров
 *
 *  Следит за папками сессий/cookies всех установленных браузеров.
 *  Если кто-то (стилер, малварь) пытается прочитать/скопировать/изменить
 *  эти файлы — программа немедленно уведомляет пользователя.
 *
 *  Поддерживает: Chrome, Edge, Firefox, Opera, Opera GX, Brave,
 *                Yandex Browser, Vivaldi
 *
 *  Сборка: cmake -B build && cmake --build build --config Release
 *  Запуск: WenzGuard.exe [--silent] [--scan-only]
 */

#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <csignal>

#include "browser_paths.h"
#include "monitor.h"
#include "alert.h"
#include "logger.h"
#include "process_scanner.h"

// ==================== ГЛОБАЛЫ ====================

static std::atomic<bool> g_running{true};
static SessionMonitor* g_monitor = nullptr;

// ==================== CTRL+C HANDLER ====================

BOOL WINAPI console_handler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        std::wcout << L"\n[WenzGuard] Остановка..." << std::endl;
        g_running.store(false);
        if (g_monitor) g_monitor->stop();
        return TRUE;
    }
    return FALSE;
}

// ==================== БАННЕР ====================

void print_banner() {
    // Установка UTF-8 консоли
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::wcout << L"\n";
    std::wcout << L"  ╦ ╦╔═╗╔╗╔╔═╗  ╔═╗╦ ╦╔═╗╦═╗╔╦╗\n";
    std::wcout << L"  ║║║║╣ ║║║╔═╝  ║ ╦║ ║╠═╣╠╦╝ ║║\n";
    std::wcout << L"  ╚╩╝╚═╝╝╚╝╚═╝  ╚═╝╚═╝╩ ╩╩╚══╩╝\n";
    std::wcout << L"\n";
    std::wcout << L"  Монитор безопасности сессий v1.0\n";
    std::wcout << L"  ─────────────────────────────────\n\n";
}

// ==================== СКАНИРОВАНИЕ ПРОЦЕССОВ ====================

void run_process_scan() {
    Logger::instance().info(L"Сканирование процессов...");

    auto suspicious = scan_suspicious_processes();

    if (suspicious.empty()) {
        Logger::instance().info(L"Подозрительных процессов не обнаружено ✓");
    } else {
        for (const auto& proc : suspicious) {
            std::wstring msg = L"⚠ ПОДОЗРИТЕЛЬНЫЙ ПРОЦЕСС: " + proc.name +
                               L" (PID: " + std::to_wstring(proc.pid) + L") — " + proc.reason;

            if (!proc.path.empty()) {
                msg += L"\n    Путь: " + proc.path;
            }

            Logger::instance().alert(msg);

            // Если это известный стилер — полная тревога
            std::wstring lower = proc.name;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
            if (KNOWN_STEALERS.count(lower) > 0) {
                play_alarm_sound();
                show_alert(
                    L"⚠ WenzGuard — ОБНАРУЖЕН СТИЛЕР!",
                    L"Обнаружен известный стилер: " + proc.name +
                    L"\nPID: " + std::to_wstring(proc.pid) +
                    L"\nПуть: " + proc.path +
                    L"\n\nНЕМЕДЛЕННО завершите этот процесс через Диспетчер задач!"
                );
            }
        }
    }
}

// ==================== ПЕРИОДИЧЕСКОЕ СКАНИРОВАНИЕ ====================

void periodic_scan_thread() {
    while (g_running.load()) {
        // Сканируем каждые 30 секунд
        for (int i = 0; i < 30 && g_running.load(); i++) {
            Sleep(1000);
        }
        if (!g_running.load()) break;

        run_process_scan();
    }
}

// ==================== MAIN ====================

int wmain(int argc, wchar_t* argv[]) {
    // Параметры командной строки
    bool silent = false;
    bool scan_only = false;

    for (int i = 1; i < argc; i++) {
        std::wstring arg = argv[i];
        if (arg == L"--silent" || arg == L"-s") silent = true;
        if (arg == L"--scan-only" || arg == L"--scan") scan_only = true;
        if (arg == L"--help" || arg == L"-h") {
            std::wcout << L"WenzGuard — Монитор безопасности сессий\n\n";
            std::wcout << L"Использование: WenzGuard.exe [параметры]\n\n";
            std::wcout << L"  --silent, -s     Без всплывающих окон (только консоль и лог)\n";
            std::wcout << L"  --scan-only      Только сканирование (без мониторинга)\n";
            std::wcout << L"  --help, -h       Эта справка\n";
            return 0;
        }
    }

    print_banner();

    // Ctrl+C handler
    SetConsoleCtrlHandler(console_handler, TRUE);

    // Инициализация логгера
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(NULL, exe_path, MAX_PATH);
    std::wstring log_path = exe_path;
    log_path = log_path.substr(0, log_path.find_last_of(L'\\') + 1) + L"wenzguard.log";
    Logger::instance().init(log_path);
    Logger::instance().info(L"WenzGuard запущен. Лог: " + log_path);

    // Обнаружение браузеров
    auto profiles = discover_browser_profiles();

    if (profiles.empty()) {
        Logger::instance().warn(L"Браузеры не найдены! Нечего мониторить.");
        std::wcout << L"  Браузеры не обнаружены на этом ПК.\n";
        return 1;
    }

    std::wcout << L"  Обнаружено профилей: " << profiles.size() << L"\n\n";

    for (const auto& p : profiles) {
        std::wcout << L"  ✓ " << p.browser_name << L"\n";
        std::wcout << L"    " << p.profile_path << L"\n";
    }
    std::wcout << L"\n";

    // Первичное сканирование процессов
    run_process_scan();

    if (scan_only) {
        std::wcout << L"\n  Сканирование завершено.\n";
        return 0;
    }

    // Настройка мониторинга
    SessionMonitor monitor;
    g_monitor = &monitor;

    for (const auto& profile : profiles) {
        monitor.add_watch(profile.profile_path, profile.browser_name);
    }

    // Счётчик событий для подавления спама
    static std::atomic<int> event_count{0};
    static auto last_alert_time = std::chrono::steady_clock::now();

    // Callback при обнаружении изменения чувствительного файла
    monitor.set_alert_callback([&](const FileEvent& event) {
        // Определяем уровень угрозы
        std::wstring lower_path = event.file_path;
        std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(), ::towlower);

        bool is_critical = lower_path.find(L"cookies") != std::wstring::npos
                        || lower_path.find(L"login data") != std::wstring::npos
                        || lower_path.find(L"local state") != std::wstring::npos
                        || lower_path.find(L"key4.db") != std::wstring::npos
                        || lower_path.find(L"logins.json") != std::wstring::npos;

        bool is_session = lower_path.find(L"session") != std::wstring::npos
                       || lower_path.find(L"tabs") != std::wstring::npos;

        // Логируем все события
        std::wstring log_msg = L"[" + event.browser_name + L"] " +
                               event.action + L": " + event.file_path;

        if (is_critical) {
            Logger::instance().alert(L"🔴 КРИТИЧНО " + log_msg);
        } else if (is_session) {
            Logger::instance().warn(L"🟡 СЕССИЯ " + log_msg);
        } else {
            Logger::instance().info(L"🔵 " + log_msg);
        }

        // Подавление спама (не больше 1 алерта в 5 секунд)
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_alert_time).count();

        if (is_critical && elapsed >= 5 && !silent) {
            last_alert_time = now;
            event_count++;

            // Звук тревоги
            play_alarm_sound();

            // Уведомление
            std::wstring alert_msg =
                L"Обнаружено подозрительное обращение к данным!\n\n"
                L"Браузер: " + event.browser_name + L"\n"
                L"Файл: " + event.file_path + L"\n"
                L"Действие: " + event.action + L"\n"
                L"Время: " + event.timestamp + L"\n\n"
                L"Если вы не открывали этот браузер — возможно, "
                L"стилер пытается украсть ваши данные!\n\n"
                L"Рекомендации:\n"
                L"1. Откройте Диспетчер задач (Ctrl+Shift+Esc)\n"
                L"2. Ищите подозрительные процессы\n"
                L"3. Смените пароли в браузере\n"
                L"4. Проверьте ПК антивирусом";

            // Показываем в отдельном потоке чтобы не блокировать мониторинг
            std::thread([alert_msg]() {
                show_alert(L"⚠ WenzGuard — ТРЕВОГА!", alert_msg);
            }).detach();
        }
    });

    // Запускаем периодическое сканирование процессов в отдельном потоке
    std::thread scan_thread(periodic_scan_thread);
    scan_thread.detach();

    std::wcout << L"  ─────────────────────────────────\n";
    std::wcout << L"  Мониторинг запущен. Ctrl+C для выхода.\n";
    std::wcout << L"  ─────────────────────────────────\n\n";

    Logger::instance().info(L"Мониторинг запущен. Отслеживается " +
                           std::to_wstring(profiles.size()) + L" профилей.");

    // Запуск мониторинга (блокирующий)
    monitor.start();

    g_monitor = nullptr;
    Logger::instance().info(L"WenzGuard остановлен.");
    return 0;
}
