#pragma once
#include <string>
#include <vector>
#include <set>
#include <unordered_set>

struct SuspiciousProcess {
    DWORD pid;
    std::wstring name;
    std::wstring path;
    std::wstring reason;
};

// Known stealer process names (lowercase)
inline const std::unordered_set<std::wstring> KNOWN_STEALERS = {
    // Popular stealers
    L"redline.exe", L"vidar.exe", L"raccoon.exe", L"mars.exe",
    L"arkei.exe", L"azorult.exe", L"predator.exe", L"kpot.exe",
    L"pony.exe", L"loki.exe", L"ficker.exe", L"danabot.exe",
    L"taurus.exe", L"stealc.exe", L"risepro.exe", L"lumma.exe",
    L"rhadamanthys.exe", L"mystic.exe", L"worldwind.exe",
    L"meduza.exe", L"atlantida.exe", L"whitesnake.exe",
    L"aurora.exe", L"titan.exe",
    // Tools often used maliciously
    L"mimikatz.exe", L"lazagne.exe", L"browserpassview.exe",
    L"webbrowserpassview.exe", L"chromepass.exe",
    L"iepv.exe", L"pspv.exe", L"mailpv.exe",
    L"netpass.exe", L"wirelesskeyview.exe",
    L"passwordfox.exe", L"operapassview.exe",
    // Grabbers
    L"dcratbuild.exe", L"asyncrat.exe", L"quasar.exe",
    L"nanocore.exe", L"njrat.exe", L"remcos.exe",
    L"orcus.exe", L"xworm.exe", L"venom.exe",
};

inline const std::unordered_set<std::wstring> BROWSER_PROCESSES = {
    L"chrome.exe", L"msedge.exe", L"firefox.exe", L"opera.exe",
    L"brave.exe", L"vivaldi.exe", L"browser.exe", L"yandex.exe",
    L"iexplore.exe", L"safari.exe",
};

inline const std::unordered_set<std::wstring> SYSTEM_PROCESSES = {
    L"system", L"system idle process", L"registry", L"smss.exe",
    L"csrss.exe", L"wininit.exe", L"services.exe", L"lsass.exe",
    L"svchost.exe", L"dwm.exe", L"explorer.exe", L"sihost.exe",
    L"taskhostw.exe", L"ctfmon.exe", L"runtimebroker.exe",
    L"searchhost.exe", L"startmenuexperiencehost.exe",
    L"textinputhost.exe", L"shellexperiencehost.exe",
    L"applicationframehost.exe", L"systemsettings.exe",
    L"securityhealthservice.exe", L"securityhealthsystray.exe",
    L"msmpeng.exe", L"nissrv.exe", L"mpcmdrun.exe",
    L"spoolsv.exe", L"audiodg.exe", L"fontdrvhost.exe",
    L"dllhost.exe", L"conhost.exe", L"cmd.exe", L"powershell.exe",
    L"windowsterminal.exe", L"wenzguard.exe",
    L"code.exe", L"devenv.exe", L"msbuild.exe", L"cmake.exe",
    L"git.exe", L"node.exe", L"npm.exe", L"python.exe",
    L"telegram.exe", L"discord.exe", L"steam.exe",
    L"steamwebhelper.exe", L"epicgameslauncher.exe",
    L"slack.exe", L"teams.exe", L"whatsapp.exe",
    L"spotify.exe", L"obs64.exe", L"obs32.exe",
    L"taskmgr.exe", L"mmc.exe", L"regedit.exe",
    L"notepad.exe", L"notepad++.exe", L"winrar.exe", L"7zfm.exe",
    L"searchindexer.exe", L"searchprotocolhost.exe",
    L"searchfilterhost.exe", L"wmiprvse.exe",
    L"backgroundtaskhost.exe", L"gamebar.exe",
    L"gamebarpresencewriter.exe", L"gamebarftserver.exe",
    L"widgets.exe", L"widgetservice.exe",
    L"lockapp.exe", L"logonui.exe", L"winlogon.exe",
    L"lsaiso.exe", L"sgrmbroker.exe", L"crashpad_handler.exe",
    L"smartscreen.exe", L"userinit.exe",
};

// Safe exe locations (lowercase) - processes from these are usually legit
inline const std::vector<std::wstring> SAFE_LOCATIONS = {
    L"\\windows\\",
    L"\\program files\\",
    L"\\program files (x86)\\",
    L"\\windowsapps\\",
    L"\\microsoft\\",
};

bool is_browser_process(const std::wstring& process_name);
bool is_suspicious_process(const std::wstring& process_name);
bool is_safe_location(const std::wstring& path);
bool is_suspicious_location(const std::wstring& path);

// One-shot scan
std::vector<SuspiciousProcess> scan_suspicious_processes();

// Get all current PIDs with their info
struct ProcessInfo {
    DWORD pid;
    std::wstring name;
    std::wstring path;
};
std::vector<ProcessInfo> get_all_processes();
