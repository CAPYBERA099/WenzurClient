#include "alert.h"
#include <windows.h>
#include <shellapi.h>

// Глобальные данные для tray icon
static NOTIFYICONDATAW g_nid = {};
static HWND g_hwnd = NULL;
static bool g_tray_initialized = false;

// Скрытое окно для приёма сообщений tray
static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static void init_tray() {
    if (g_tray_initialized) return;

    // Регистрируем класс окна
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = L"WenzGuardTray";
    RegisterClassExW(&wc);

    // Создаём скрытое окно
    g_hwnd = CreateWindowExW(
        0, L"WenzGuardTray", L"WenzGuard", 0,
        0, 0, 0, 0, HWND_MESSAGE, NULL, GetModuleHandleW(NULL), NULL
    );

    // Добавляем иконку в трей
    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = g_hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    g_nid.uCallbackMessage = WM_USER + 1;
    g_nid.hIcon = LoadIconW(NULL, IDI_SHIELD);
    wcscpy_s(g_nid.szTip, L"WenzGuard - Session Monitor");

    Shell_NotifyIconW(NIM_ADD, &g_nid);
    g_tray_initialized = true;
}

void show_notification(const std::wstring& title, const std::wstring& message) {
    init_tray();

    NOTIFYICONDATAW nid = g_nid;
    nid.uFlags = NIF_INFO;
    nid.dwInfoFlags = NIIF_WARNING;

    // Копируем title (макс 63 символа)
    wcsncpy_s(nid.szInfoTitle, title.c_str(), 63);

    // Копируем message (макс 255 символов)
    wcsncpy_s(nid.szInfo, message.c_str(), 255);

    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

void play_alarm_sound() {
    MessageBeep(MB_ICONEXCLAMATION);
}
