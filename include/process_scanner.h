#pragma once
#include <string>
#include <vector>
#include <set>

struct SuspiciousProcess {
    DWORD pid;
    std::wstring name;
    std::wstring path;
    std::wstring reason;
};

// Список легитимных процессов браузеров (не вызывают тревогу)
const std::set<std::wstring> BROWSER_PROCESSES = {
    L"chrome.exe",
    L"firefox.exe",
    L"msedge.exe",
    L"opera.exe",
    L"brave.exe",
    L"browser.exe",      // Yandex Browser
    L"vivaldi.exe",
    L"iridium.exe",
};

// Системные процессы которые могут легитимно читать файлы
const std::set<std::wstring> SYSTEM_PROCESSES = {
    L"svchost.exe",
    L"system",
    L"searchindexer.exe",
    L"explorer.exe",
    L"smartscreen.exe",
    L"mpcmdrun.exe",        // Windows Defender
    L"msmpeng.exe",         // Windows Defender
    L"securityhealthservice.exe",
};

// Известные стилеры и подозрительные процессы
const std::set<std::wstring> KNOWN_STEALERS = {
    L"redline.exe",
    L"vidar.exe",
    L"raccoon.exe",
    L"azorult.exe",
    L"predator.exe",
    L"kpot.exe",
    L"ficker.exe",
    L"mars.exe",
    L"meta.exe",
    L"stealc.exe",
    L"lumma.exe",
    L"rhadamanthys.exe",
    L"risepro.exe",
};

// Сканировать процессы, обращающиеся к файлам браузера
std::vector<SuspiciousProcess> scan_suspicious_processes();

// Проверить, запущен ли подозрительный процесс
bool is_suspicious_process(const std::wstring& process_name);

// Проверить, является ли процесс легитимным браузером
bool is_browser_process(const std::wstring& process_name);
