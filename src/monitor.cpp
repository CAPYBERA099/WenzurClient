#include "monitor.h"
#include "logger.h"
#include <windows.h>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

// Файлы которые вызывают тревогу при изменении
static const std::vector<std::wstring> CRITICAL_FILES = {
    // Browsers
    L"cookies", L"login data", L"web data", L"local state",
    L"cookies-journal", L"login data-journal",
    L"cookies.sqlite", L"logins.json", L"key4.db", L"key3.db",
    L"extension cookies", L"sessionstore.jsonlz4",
    // Telegram
    L"key_datas", L"key_data",
    // Discord (only .ldb in discord paths — checked via browser_name)
    L".ldb",
    // Steam
    L"ssfn", L"loginusers.vdf",
    // Minecraft
    L"launcher_accounts.json",
    // Crypto
    L"seed.seco", L"seed.json", L"default_wallet",
    // FTP/SSH
    L"recentservers.xml", L"sitemanager.xml",
    // Git
    L".git-credentials",
    // Honeypot canaries (always match)
    L".wgcanary", L"canary",
};

SessionMonitor::SessionMonitor() {}

SessionMonitor::~SessionMonitor() {
    stop();
}

void SessionMonitor::add_watch(const std::wstring& path, const std::wstring& browser_name) {
    if (fs::exists(path)) {
        watch_paths_.push_back({ path, browser_name });
    }
}

void SessionMonitor::set_alert_callback(AlertCallback callback) {
    alert_callback_ = callback;
}

void SessionMonitor::start() {
    running_.store(true);

    for (auto& [path, name] : watch_paths_) {
        threads_.emplace_back(&SessionMonitor::watch_directory, this, path, name);
    }

    for (auto& t : threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void SessionMonitor::stop() {
    running_.store(false);
}

bool SessionMonitor::is_sensitive_file(const std::wstring& filename) {
    std::wstring lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);

    for (const auto& critical : CRITICAL_FILES) {
        if (lower.find(critical) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

std::wstring SessionMonitor::get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    struct tm tm_buf;
    localtime_s(&tm_buf, &time);

    std::wostringstream oss;
    oss << std::put_time(&tm_buf, L"%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void SessionMonitor::watch_directory(const std::wstring& path, const std::wstring& browser_name) {
    HANDLE hDir = CreateFileW(
        path.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (hDir == INVALID_HANDLE_VALUE) {
        Logger::instance().error(L"Не удалось открыть: " + path);
        return;
    }

    Logger::instance().info(L"Мониторинг: " + browser_name + L" -> " + path);

    const DWORD buf_size = 65536;
    auto buffer = std::make_unique<BYTE[]>(buf_size);

    OVERLAPPED overlapped = {};
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    while (running_.load()) {
        BOOL result = ReadDirectoryChangesW(
            hDir,
            buffer.get(),
            buf_size,
            TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_LAST_WRITE |
            FILE_NOTIFY_CHANGE_SIZE |
            FILE_NOTIFY_CHANGE_ATTRIBUTES |
            FILE_NOTIFY_CHANGE_CREATION,
            NULL,
            &overlapped,
            NULL
        );

        if (!result && GetLastError() != ERROR_IO_PENDING) {
            Logger::instance().error(L"ReadDirectoryChangesW ошибка: " + std::to_wstring(GetLastError()));
            break;
        }

        DWORD wait_result = WaitForSingleObject(overlapped.hEvent, 2000);

        if (wait_result == WAIT_TIMEOUT) continue;
        if (wait_result != WAIT_OBJECT_0) continue;

        DWORD bytes_returned = 0;
        if (!GetOverlappedResult(hDir, &overlapped, &bytes_returned, FALSE) || bytes_returned == 0) {
            ResetEvent(overlapped.hEvent);
            continue;
        }

        FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)buffer.get();

        do {
            std::wstring filename(fni->FileName, fni->FileNameLength / sizeof(wchar_t));

            if (is_sensitive_file(filename)) {
                std::wstring action;
                switch (fni->Action) {
                    case FILE_ACTION_ADDED:            action = L"CREATED"; break;
                    case FILE_ACTION_REMOVED:          action = L"DELETED"; break;
                    case FILE_ACTION_MODIFIED:         action = L"MODIFIED"; break;
                    case FILE_ACTION_RENAMED_OLD_NAME: action = L"RENAMED"; break;
                    case FILE_ACTION_RENAMED_NEW_NAME: action = L"RENAMED"; break;
                    default:                           action = L"ACCESSED";
                }

                FileEvent event;
                event.file_path = path + L"\\" + filename;
                event.action = action;
                event.browser_name = browser_name;
                event.timestamp = get_timestamp();

                if (alert_callback_) {
                    alert_callback_(event);
                }
            }

            if (fni->NextEntryOffset == 0) break;
            fni = (FILE_NOTIFY_INFORMATION*)((BYTE*)fni + fni->NextEntryOffset);
        } while (true);

        ResetEvent(overlapped.hEvent);
    }

    CloseHandle(overlapped.hEvent);
    CloseHandle(hDir);
}
