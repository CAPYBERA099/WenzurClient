#include "honeypot.h"
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <cstdio>

namespace fs = std::filesystem;

static std::wstring get_local_appdata() {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path) == S_OK)
        return path;
    return L"";
}

static std::wstring get_appdata() {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path) == S_OK)
        return path;
    return L"";
}

// Create a canary file with fake data that looks real
static bool create_canary(const std::wstring& dir, const std::wstring& filename,
                          const std::string& fake_content) {
    std::wstring full = dir + L"\\" + filename;

    // Don't overwrite real files!
    if (fs::exists(full)) return false;

    // Make sure directory exists
    if (!fs::exists(dir)) return false;

    std::ofstream f(full, std::ios::binary);
    if (!f.is_open()) return false;

    f.write(fake_content.data(), fake_content.size());
    f.close();

    // Set file as hidden + system so user doesn't see it
    SetFileAttributesW(full.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

    return true;
}

std::vector<HoneypotFile> deploy_honeypots() {
    std::vector<HoneypotFile> honeypots;
    std::wstring local = get_local_appdata();
    std::wstring roaming = get_appdata();

    // Fake content that looks like real data
    std::string fake_cookie = "SQLite format 3\x00\x10\x00\x01\x01\x00\x40\x20"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "WENZGUARD_CANARY_DO_NOT_STEAL";

    std::string fake_login = "SQLite format 3\x00\x10\x00\x01\x01\x00\x40\x20"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "WENZGUARD_CANARY_LOGIN_TRAP";

    std::string fake_ldb = "WENZGUARD_CANARY_TOKEN_TRAP_v1\n"
        "dQw4w9WgXcQ.faketoken.canary\n";

    std::string fake_tdata = "WENZGUARD_CANARY_TDATA_TRAP\x00\x00\x00\x00";

    struct {
        std::wstring dir;
        std::wstring filename;
        std::wstring app;
        std::wstring dtype;
        std::string content;
    } canaries[] = {
        // Chrome — fake cookie file in Network subfolder
        { local + L"\\Google\\Chrome\\User Data\\Default",
          L"Cookies.wgcanary", L"Chrome", L"COOKIES", fake_cookie },

        // Chrome — fake login
        { local + L"\\Google\\Chrome\\User Data\\Default",
          L"Login Data.wgcanary", L"Chrome", L"PASSWORDS", fake_login },

        // Edge
        { local + L"\\Microsoft\\Edge\\User Data\\Default",
          L"Cookies.wgcanary", L"Edge", L"COOKIES", fake_cookie },

        // Opera
        { roaming + L"\\Opera Software\\Opera Stable",
          L"Cookies.wgcanary", L"Opera", L"COOKIES", fake_cookie },

        // Brave
        { local + L"\\BraveSoftware\\Brave-Browser\\User Data\\Default",
          L"Cookies.wgcanary", L"Brave", L"COOKIES", fake_cookie },

        // Yandex
        { local + L"\\Yandex\\YandexBrowser\\User Data\\Default",
          L"Cookies.wgcanary", L"Yandex Browser", L"COOKIES", fake_cookie },

        // Discord — fake ldb
        { roaming + L"\\discord\\Local Storage\\leveldb",
          L"CANARY.ldb", L"Discord", L"DISCORD TOKEN", fake_ldb },

        // Discord Canary
        { roaming + L"\\discordcanary\\Local Storage\\leveldb",
          L"CANARY.ldb", L"Discord Canary", L"DISCORD TOKEN", fake_ldb },

        // Telegram — fake tdata
        { roaming + L"\\Telegram Desktop\\tdata",
          L"wg_canary_key", L"Telegram", L"TELEGRAM SESSION", fake_tdata },

        // Steam config
        { L"C:\\Program Files (x86)\\Steam\\config",
          L"canary_ssfn", L"Steam", L"STEAM SESSION", fake_tdata },
    };

    printf("  Honeypots:\n");

    for (auto& c : canaries) {
        if (create_canary(c.dir, c.filename, c.content)) {
            HoneypotFile hp;
            hp.path = c.dir + L"\\" + c.filename;
            hp.app_name = c.app;
            hp.data_type = c.dtype;
            honeypots.push_back(hp);
            printf("    [TRAP] %ls -> %ls\n", c.app.c_str(), c.filename.c_str());
        }
    }

    if (honeypots.empty()) {
        printf("    No honeypots deployed (no app dirs found)\n");
    } else {
        printf("    %d canary files deployed\n", (int)honeypots.size());
    }

    return honeypots;
}

void cleanup_honeypots(const std::vector<HoneypotFile>& honeypots) {
    for (const auto& hp : honeypots) {
        try {
            // Remove hidden+system attributes first
            SetFileAttributesW(hp.path.c_str(), FILE_ATTRIBUTE_NORMAL);
            fs::remove(hp.path);
        } catch (...) {}
    }
}
