#pragma once
#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
    static Logger& instance();

    void init(const std::wstring& log_path);
    void log(const std::wstring& level, const std::wstring& message);
    void info(const std::wstring& message);
    void warn(const std::wstring& message);
    void alert(const std::wstring& message);
    void error(const std::wstring& message);

private:
    Logger() = default;
    std::wofstream file_;
    std::mutex mutex_;
    std::wstring get_timestamp();
};
