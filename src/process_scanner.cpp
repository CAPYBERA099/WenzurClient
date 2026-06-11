#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include "process_scanner.h"
#include <algorithm>
#include <cstdio>

bool is_browser_process(const std::wstring& process_name) {
    std::wstring lower = process_name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
    return BROWSER_PROCESSES.count(lower) > 0;
}

bool is_suspicious_process(const std::wstring& process_name) {
    std::wstring lower = process_name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);

    if (KNOWN_STEALERS.count(lower) > 0) return true;

    const std::vector<std::wstring> suspicious_patterns = {
        L"stealer", L"steal", L"grab", L"dump", L"inject",
        L"keylog", L"spy", L"rat", L"trojan", L"hack",
        L"cookie", L"passwd", L"cred", L"token",
        L"loader", L"dropper", L"crypter", L"clipper",
        L"miner", L"xmrig", L"brute",
    };

    for (const auto& pattern : suspicious_patterns) {
        if (lower.find(pattern) != std::wstring::npos) return true;
    }

    return false;
}

std::vector<SuspiciousProcess> scan_suspicious_processes() {
    std::vector<SuspiciousProcess> results;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return results;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(snapshot, &pe)) {
        do {
            std::wstring name = pe.szExeFile;
            std::wstring lower = name;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);

            // Skip known safe processes
            if (is_browser_process(name)) continue;
            if (SYSTEM_PROCESSES.count(lower) > 0) continue;

            // Get process path
            std::wstring proc_path;
            HANDLE hProcess = OpenProcess(
                PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                FALSE, pe.th32ProcessID
            );
            if (hProcess) {
                wchar_t path[MAX_PATH];
                if (GetModuleFileNameExW(hProcess, NULL, path, MAX_PATH)) {
                    proc_path = path;
                }
                CloseHandle(hProcess);
            }

            std::wstring path_lower = proc_path;
            std::transform(path_lower.begin(), path_lower.end(), path_lower.begin(), ::towlower);

            // Check 1: Known stealer name
            if (KNOWN_STEALERS.count(lower) > 0) {
                results.push_back({
                    pe.th32ProcessID, name, proc_path,
                    L"KNOWN STEALER"
                });
                continue;
            }

            // Check 2: Suspicious name pattern
            if (is_suspicious_process(name)) {
                results.push_back({
                    pe.th32ProcessID, name, proc_path,
                    L"Suspicious process name"
                });
                continue;
            }

            // Check 3: Running from Temp/Downloads (potential malware)
            if (!proc_path.empty()) {
                bool from_temp = path_lower.find(L"\\temp\\") != std::wstring::npos
                              || path_lower.find(L"\\tmp\\") != std::wstring::npos;
                bool from_downloads = path_lower.find(L"\\downloads\\") != std::wstring::npos;
                bool from_appdata_local_temp = path_lower.find(L"\\appdata\\local\\temp\\") != std::wstring::npos;

                // Exclude known safe apps from these folders
                bool safe_name = lower.find(L"setup") != std::wstring::npos
                              || lower.find(L"install") != std::wstring::npos
                              || lower.find(L"update") != std::wstring::npos
                              || lower == L"discord.exe"
                              || lower == L"code.exe";

                if ((from_temp || from_downloads || from_appdata_local_temp) && !safe_name) {
                    std::wstring reason = L"Running from ";
                    if (from_appdata_local_temp || from_temp) reason += L"Temp folder";
                    else reason += L"Downloads folder";

                    results.push_back({
                        pe.th32ProcessID, name, proc_path, reason
                    });
                }
            }

        } while (Process32NextW(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return results;
}
