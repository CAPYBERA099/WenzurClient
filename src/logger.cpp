#include "logger.h"
#include <windows.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::init(const std::wstring& log_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    file_.open(log_path, std::ios::app | std::ios::out);
    if (file_.is_open()) {
        file_ << L"\n========================================\n";
        file_ << L"WenzGuard запущен: " << get_timestamp() << L"\n";
        file_ << L"========================================\n";
        file_.flush();
    }
}

std::wstring Logger::get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    struct tm tm_buf;
    localtime_s(&tm_buf, &time);

    std::wostringstream oss;
    oss << std::put_time(&tm_buf, L"%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Logger::log(const std::wstring& level, const std::wstring& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::wstring line = L"[" + get_timestamp() + L"] [" + level + L"] " + message;

    // Консоль
    std::wcout << line << std::endl;

    // Файл
    if (file_.is_open()) {
        file_ << line << std::endl;
        file_.flush();
    }
}

void Logger::info(const std::wstring& message) {
    log(L"INFO", message);
}

void Logger::warn(const std::wstring& message) {
    log(L"WARN", message);
}

void Logger::alert(const std::wstring& message) {
    log(L"ALERT", message);
}

void Logger::error(const std::wstring& message) {
    log(L"ERROR", message);
}
