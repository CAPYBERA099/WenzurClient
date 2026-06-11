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
        L"miner", L"xmrig", L"brute", L"crack",
        L"cheat", L"exploit", L"payload",
    };

    for (const auto& pattern : suspicious_patterns) {
        if (lower.find(pattern) != std::wstring::npos) return true;
    }

    return false;
}

bool is_safe_location(const std::wstring& path) {
    std::wstring lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);

    for (const auto& safe : SAFE_LOCATIONS) {
        if (lower.find(safe) != std::wstring::npos) return true;
    }
    return false;
}

bool is_suspicious_location(const std::wstring& path) {
    std::wstring lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);

    return lower.find(L"\\temp\\") != std::wstring::npos
        || lower.find(L"\\tmp\\") != std::wstring::npos
        || lower.find(L"\\downloads\\") != std::wstring::npos
        || lower.find(L"\\appdata\\local\\temp\\") != std::wstring::npos
        || lower.find(L"\\desktop\\") != std::wstring::npos;
}

std::vector<ProcessInfo> get_all_processes() {
    std::vector<ProcessInfo> results;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return results;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(snapshot, &pe)) {
        do {
            ProcessInfo pi;
            pi.pid = pe.th32ProcessID;
            pi.name = pe.szExeFile;

            // Get process path
            HANDLE hProcess = OpenProcess(
                PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                FALSE, pe.th32ProcessID
            );
            if (hProcess) {
                wchar_t path[MAX_PATH];
                if (GetModuleFileNameExW(hProcess, NULL, path, MAX_PATH)) {
                    pi.path = path;
                }
                CloseHandle(hProcess);
            }

            results.push_back(pi);
        } while (Process32NextW(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return results;
}

std::vector<SuspiciousProcess> scan_suspicious_processes() {
    std::vector<SuspiciousProcess> results;
    auto all = get_all_processes();

    for (const auto& proc : all) {
        std::wstring lower_name = proc.name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::towlower);

        // Skip known safe
        if (is_browser_process(proc.name)) continue;
        if (SYSTEM_PROCESSES.count(lower_name) > 0) continue;

        // Check 1: Known stealer
        if (KNOWN_STEALERS.count(lower_name) > 0) {
            results.push_back({
                proc.pid, proc.name, proc.path,
                L"KNOWN STEALER"
            });
            continue;
        }

        // Check 2: Suspicious name pattern
        if (is_suspicious_process(proc.name)) {
            results.push_back({
                proc.pid, proc.name, proc.path,
                L"Suspicious name"
            });
            continue;
        }

        // Check 3: Running from suspicious location
        if (!proc.path.empty() && is_suspicious_location(proc.path)
            && !is_safe_location(proc.path)) {
            results.push_back({
                proc.pid, proc.name, proc.path,
                L"From Temp/Downloads"
            });
        }
    }

    return results;
}
