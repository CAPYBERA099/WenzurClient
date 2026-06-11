#pragma once
#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include <map>

struct FileEvent {
    std::wstring file_path;
    std::wstring action;       // "MODIFIED", "READ", "DELETED", "RENAMED"
    std::wstring browser_name;
    std::wstring timestamp;
};

using AlertCallback = std::function<void(const FileEvent&)>;

class SessionMonitor {
public:
    SessionMonitor();
    ~SessionMonitor();

    // Добавить директорию для мониторинга
    void add_watch(const std::wstring& path, const std::wstring& browser_name);

    // Установить callback для алертов
    void set_alert_callback(AlertCallback callback);

    // Запустить мониторинг (блокирующий вызов)
    void start();

    // Остановить мониторинг
    void stop();

    bool is_running() const { return running_.load(); }

private:
    void watch_directory(const std::wstring& path, const std::wstring& browser_name);
    bool is_sensitive_file(const std::wstring& filename);
    std::wstring get_timestamp();

    std::vector<std::pair<std::wstring, std::wstring>> watch_paths_;
    AlertCallback alert_callback_;
    std::atomic<bool> running_{false};
    std::vector<std::thread> threads_;
};
