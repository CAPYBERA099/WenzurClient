#include "browser_paths.h"
#include <windows.h>
#include <shlobj.h>
#include <filesystem>

namespace fs = std::filesystem;

// ==================== ЧУВСТВИТЕЛЬНЫЕ ФАЙЛЫ ====================

static const std::vector<std::wstring> CHROMIUM_SENSITIVE = {
    L"Cookies", L"Cookies-journal",
    L"Login Data", L"Login Data-journal",
    L"Web Data", L"Web Data-journal",
    L"Local State", L"History", L"Bookmarks", L"Preferences",
    L"Current Session", L"Current Tabs", L"Last Session", L"Last Tabs",
    L"Extension Cookies", L"Extension Cookies-journal",
    L"Network\\Cookies", L"Network\\Cookies-journal",
};

static const std::vector<std::wstring> FIREFOX_SENSITIVE = {
    L"cookies.sqlite", L"cookies.sqlite-wal",
    L"logins.json", L"key4.db", L"key3.db", L"cert9.db",
    L"signons.sqlite", L"formhistory.sqlite", L"places.sqlite",
    L"sessionstore.jsonlz4", L"recovery.jsonlz4",
};

// ==================== УТИЛИТЫ ====================

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

static std::wstring get_userprofile() {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path) == S_OK)
        return path;
    return L"";
}

static std::wstring get_program_files() {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES, NULL, 0, path) == S_OK)
        return path;
    return L"";
}

static std::wstring get_program_files_x86() {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, path) == S_OK)
        return path;
    return L"";
}

// ==================== БРАУЗЕРЫ ====================

static void add_chromium_profile(
    std::vector<BrowserProfile>& profiles,
    const std::wstring& name,
    const std::wstring& base_path
) {
    if (!fs::exists(base_path)) return;

    std::wstring default_path = base_path + L"\\Default";
    if (fs::exists(default_path)) {
        profiles.push_back({ name, default_path, CHROMIUM_SENSITIVE });
    }

    try {
        for (auto& entry : fs::directory_iterator(base_path)) {
            std::wstring dirname = entry.path().filename().wstring();
            if (dirname.find(L"Profile ") == 0 && fs::is_directory(entry)) {
                profiles.push_back({
                    name + L" (" + dirname + L")",
                    entry.path().wstring(),
                    CHROMIUM_SENSITIVE
                });
            }
        }
    } catch (...) {}

    if (fs::exists(base_path + L"\\Local State")) {
        profiles.push_back({
            name + L" [Master Key]",
            base_path,
            { L"Local State" }
        });
    }
}

std::vector<BrowserProfile> discover_browser_profiles() {
    std::vector<BrowserProfile> profiles;
    std::wstring local = get_local_appdata();
    std::wstring roaming = get_appdata();
    if (local.empty()) return profiles;

    add_chromium_profile(profiles, L"Chrome", local + L"\\Google\\Chrome\\User Data");
    add_chromium_profile(profiles, L"Edge", local + L"\\Microsoft\\Edge\\User Data");
    add_chromium_profile(profiles, L"Opera", roaming + L"\\Opera Software\\Opera Stable");
    add_chromium_profile(profiles, L"Opera GX", roaming + L"\\Opera Software\\Opera GX Stable");
    add_chromium_profile(profiles, L"Brave", local + L"\\BraveSoftware\\Brave-Browser\\User Data");
    add_chromium_profile(profiles, L"Yandex Browser", local + L"\\Yandex\\YandexBrowser\\User Data");
    add_chromium_profile(profiles, L"Vivaldi", local + L"\\Vivaldi\\User Data");

    // Firefox
    std::wstring firefox_base = roaming + L"\\Mozilla\\Firefox\\Profiles";
    if (fs::exists(firefox_base)) {
        try {
            for (auto& entry : fs::directory_iterator(firefox_base)) {
                if (fs::is_directory(entry)) {
                    profiles.push_back({
                        L"Firefox (" + entry.path().filename().wstring() + L")",
                        entry.path().wstring(),
                        FIREFOX_SENSITIVE
                    });
                }
            }
        } catch (...) {}
    }

    return profiles;
}

// ==================== ПРОГРАММЫ И МЕССЕНДЖЕРЫ ====================

static void try_add_app(
    std::vector<AppProfile>& apps,
    const std::wstring& name,
    const std::wstring& path,
    const std::vector<std::wstring>& files,
    const std::wstring& category
) {
    if (fs::exists(path)) {
        apps.push_back({ name, path, files, category });
    }
}

std::vector<AppProfile> discover_app_profiles() {
    std::vector<AppProfile> apps;
    std::wstring local = get_local_appdata();
    std::wstring roaming = get_appdata();
    std::wstring home = get_userprofile();
    std::wstring pf = get_program_files();
    std::wstring pf86 = get_program_files_x86();

    // =============== МЕССЕНДЖЕРЫ ===============

    // Telegram Desktop — tdata (самое важное!)
    try_add_app(apps, L"Telegram Desktop", roaming + L"\\Telegram Desktop\\tdata", {
        L"key_datas", L"key_data", L"D877F783D5D3EF8C",
        L"D877F783D5D3EF8Cs", L"A7FDF864FBC10B77", L"A7FDF864FBC10B77s",
        L"map0", L"map1", L"configs", L"settingss",
    }, L"messenger");

    // Discord — токены
    try_add_app(apps, L"Discord", roaming + L"\\discord\\Local Storage\\leveldb", {
        L".ldb", L".log", L"CURRENT", L"MANIFEST", L"LOG",
    }, L"messenger");

    // Discord Canary
    try_add_app(apps, L"Discord Canary", roaming + L"\\discordcanary\\Local Storage\\leveldb", {
        L".ldb", L".log", L"CURRENT", L"MANIFEST",
    }, L"messenger");

    // Discord PTB
    try_add_app(apps, L"Discord PTB", roaming + L"\\discordptb\\Local Storage\\leveldb", {
        L".ldb", L".log", L"CURRENT", L"MANIFEST",
    }, L"messenger");

    // WhatsApp Desktop
    try_add_app(apps, L"WhatsApp", local + L"\\Packages\\5319275A.WhatsAppDesktop_cv1g1gvanyjgm\\LocalState", {
        L"profile", L"databases",
    }, L"messenger");

    // Signal Desktop
    try_add_app(apps, L"Signal", roaming + L"\\Signal", {
        L"config.json", L"sql\\db.sqlite",
    }, L"messenger");

    // Viber
    try_add_app(apps, L"Viber", roaming + L"\\ViberPC", {
        L"config", L"viber.db",
    }, L"messenger");

    // Skype
    try_add_app(apps, L"Skype", roaming + L"\\Microsoft\\Skype for Desktop\\Local Storage\\leveldb", {
        L".ldb", L".log", L"CURRENT",
    }, L"messenger");

    // Element (Matrix)
    try_add_app(apps, L"Element", roaming + L"\\Element\\Local Storage\\leveldb", {
        L".ldb", L".log", L"CURRENT",
    }, L"messenger");

    // ICQ
    try_add_app(apps, L"ICQ", roaming + L"\\ICQ", {
        L"auth_token",
    }, L"messenger");

    // =============== ИГРОВЫЕ ПЛАТФОРМЫ ===============

    // Steam — ssfn файлы и config
    try_add_app(apps, L"Steam", pf86 + L"\\Steam", {
        L"ssfn*", L"config\\loginusers.vdf", L"config\\config.vdf",
        L"config\\SteamAppData.vdf",
    }, L"gaming");

    // Также Steam в Program Files
    try_add_app(apps, L"Steam", pf + L"\\Steam", {
        L"ssfn*", L"config\\loginusers.vdf", L"config\\config.vdf",
    }, L"gaming");

    // Steam — config в отдельной папке
    try_add_app(apps, L"Steam Config", local + L"\\Steam", {
        L"htmlcache", L"ssfn",
    }, L"gaming");

    // Epic Games
    try_add_app(apps, L"Epic Games", local + L"\\EpicGamesLauncher\\Saved\\Config\\Windows", {
        L"GameUserSettings.ini",
    }, L"gaming");

    // Minecraft (официальный)
    try_add_app(apps, L"Minecraft", roaming + L"\\.minecraft", {
        L"launcher_accounts.json", L"launcher_profiles.json",
    }, L"gaming");

    // Riot Games (Valorant/LoL)
    try_add_app(apps, L"Riot Client", local + L"\\Riot Games\\Riot Client\\Data", {
        L"RiotGamesPrivateSettings.yaml", L"RiotClientPrivateSettings.yaml",
    }, L"gaming");

    // Battle.net
    try_add_app(apps, L"Battle.net", roaming + L"\\Battle.net", {
        L"Battle.net.config",
    }, L"gaming");

    // Ubisoft Connect
    try_add_app(apps, L"Ubisoft Connect", local + L"\\Ubisoft Game Launcher", {
        L"settings.yml", L"user.dat",
    }, L"gaming");

    // EA App
    try_add_app(apps, L"EA App", local + L"\\Electronic Arts\\EA Desktop", {
        L"user_*.ini",
    }, L"gaming");

    // =============== КРИПТО / КОШЕЛЬКИ ===============

    // Exodus
    try_add_app(apps, L"Exodus Wallet", roaming + L"\\Exodus\\exodus.wallet", {
        L"seed.seco", L"passphrase.json", L"seed.json", L"info.seco",
    }, L"crypto");

    // Atomic Wallet
    try_add_app(apps, L"Atomic Wallet", roaming + L"\\atomic\\Local Storage\\leveldb", {
        L".ldb", L".log",
    }, L"crypto");

    // Electrum
    try_add_app(apps, L"Electrum", roaming + L"\\Electrum\\wallets", {
        L"default_wallet",
    }, L"crypto");

    // MetaMask (расширение Chrome)
    // nkbihfbeogaeaoehlefnkodbefgpgknn — ID расширения MetaMask
    try_add_app(apps, L"MetaMask (Chrome)", local + L"\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\nkbihfbeogaeaoehlefnkodbefgpgknn", {
        L".ldb", L".log", L"CURRENT",
    }, L"crypto");

    // =============== VPN / ПРОКСИ ===============

    // NordVPN
    try_add_app(apps, L"NordVPN", local + L"\\NordVPN", {
        L"NordVpn.exe.config", L"user.config",
    }, L"vpn");

    // ProtonVPN
    try_add_app(apps, L"ProtonVPN", local + L"\\ProtonVPN", {
        L"user.config",
    }, L"vpn");

    // OpenVPN
    try_add_app(apps, L"OpenVPN", roaming + L"\\OpenVPN Connect\\profiles", {
        L"*.ovpn",
    }, L"vpn");

    // =============== ДРУГИЕ ПРИЛОЖЕНИЯ ===============

    // FileZilla — FTP пароли
    try_add_app(apps, L"FileZilla", roaming + L"\\FileZilla", {
        L"recentservers.xml", L"sitemanager.xml", L"filezilla.xml",
    }, L"other");

    // WinSCP — SSH/SCP пароли
    try_add_app(apps, L"WinSCP", roaming + L"\\WinSCP", {
        L"WinSCP.ini",
    }, L"other");

    // Outlook
    try_add_app(apps, L"Outlook", local + L"\\Microsoft\\Outlook", {
        L"*.ost", L"*.pst",
    }, L"other");

    // Thunderbird
    std::wstring tb_base = roaming + L"\\Thunderbird\\Profiles";
    if (fs::exists(tb_base)) {
        try {
            for (auto& entry : fs::directory_iterator(tb_base)) {
                if (fs::is_directory(entry)) {
                    apps.push_back({
                        L"Thunderbird (" + entry.path().filename().wstring() + L")",
                        entry.path().wstring(),
                        { L"logins.json", L"key4.db", L"cert9.db", L"cookies.sqlite" },
                        L"other"
                    });
                }
            }
        } catch (...) {}
    }

    // Authy Desktop
    try_add_app(apps, L"Authy", roaming + L"\\Authy Desktop\\Local Storage\\leveldb", {
        L".ldb", L".log",
    }, L"other");

    // OBS Studio (stream key)
    try_add_app(apps, L"OBS Studio", roaming + L"\\obs-studio\\basic\\profiles", {
        L"service.json",
    }, L"other");

    // Git credentials — watch ONLY the file itself, not the whole home dir
    if (fs::exists(home + L"\\.git-credentials")) {
        // Don't add the whole home directory — it would catch ALL file changes!
        // Instead skip this; the honeypot + process scanner handles it.
    }

    return apps;
}
