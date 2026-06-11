#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include "process_scanner.h"
#include "logger.h"
#include <algorithm>

bool is_browser_process(const std::wstring& process_name) {
    std::wstring lower = process_name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
    return BROWSER_PROCESSES.count(lower) > 0;
}

bool is_suspicious_process(const std::wstring& process_name) {
    std::wstring lower = process_name;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);

    // Прямое совпадение с известными стилерами
    if (KNOWN_STEALERS.count(lower) > 0) return true;

    // Подозрительные паттерны в имени процесса
    const std::vector<std::wstring> suspicious_patterns = {
        L"stealer", L"steal", L"grab", L"dump", L"inject",
        L"keylog", L"spy", L"rat", L"trojan", L"hack",
        L"cookie", L"passwd", L"cred", L"token",
        L"loader", L"dropper", L"crypter",
    };

    for (const auto& pattern : suspicious_patterns) {
        if (lower.find(pattern) != std::wstring::npos) {
            return true;
        }
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

            // Пропускаем легитимные процессы
            if (is_browser_process(name)) continue;

            std::wstring lower = name;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
            if (SYSTEM_PROCESSES.count(lower) > 0) continue;

            // Проверяем на подозрительность
            if (is_suspicious_process(name)) {
                SuspiciousProcess sp;
                sp.pid = pe.th32ProcessID;
                sp.name = name;
                sp.reason = L"Подозрительное имя процесса";

                // Получаем путь к exe
                HANDLE hProcess = OpenProcess(
                    PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                    FALSE, pe.th32ProcessID
                );
                if (hProcess) {
                    wchar_t path[MAX_PATH];
                    if (GetModuleFileNameExW(hProcess, NULL, path, MAX_PATH)) {
                        sp.path = path;
                    }
                    CloseHandle(hProcess);
                }

                // Проверяем, из известных ли стилеров
                if (KNOWN_STEALERS.count(lower) > 0) {
                    sp.reason = L"ИЗВЕСТНЫЙ СТИЛЕР!";
                }

                results.push_back(sp);
            }

            // Подозрительно: процесс запущен из Temp
            HANDLE hProcess = OpenProcess(
                PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                FALSE, pe.th32ProcessID
            );
            if (hProcess) {
                wchar_t path[MAX_PATH];
                if (GetModuleFileNameExW(hProcess, NULL, path, MAX_PATH)) {
                    std::wstring proc_path = path;
                    std::transform(proc_path.begin(), proc_path.end(), proc_path.begin(), ::towlower);

                    bool from_temp = proc_path.find(L"\\temp\\") != std::wstring::npos
                                  || proc_path.find(L"\\tmp\\") != std::wstring::npos;
                    bool from_downloads = proc_path.find(L"\\downloads\\") != std::wstring::npos;

                    if ((from_temp || from_downloads) && !is_browser_process(name)) {
                        // Не добавляем дубли
                        bool already = false;
                        for (auto& r : results) {
                            if (r.pid == pe.th32ProcessID) { already = true; break; }
                        }
                        if (!already) {
                            SuspiciousProcess sp;
                            sp.pid = pe.th32ProcessID;
                            sp.name = name;
                            sp.path = path;
                            sp.reason = from_temp
                                ? L"Запущен из папки Temp"
                                : L"Запущен из папки Downloads";
                            results.push_back(sp);
                        }
                    }
                }
                CloseHandle(hProcess);
            }

        } while (Process32NextW(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return results;
}
