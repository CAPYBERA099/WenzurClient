#include "alert.h"
#include <windows.h>
#include <shellapi.h>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <cstdio>

// ============ Notification Queue ============
// Tray icon must be created and modified from the same thread.
// We use a dedicated thread with a message pump.

struct NotifMessage {
    std::wstring title;
    std::wstring body;
};

static std::mutex g_queue_mutex;
static std::queue<NotifMessage> g_queue;
static HWND g_msg_hwnd = NULL;
static bool g_thread_started = false;

#define WM_WENZGUARD_NOTIFY (WM_USER + 100)

static NOTIFYICONDATAW g_nid = {};

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_WENZGUARD_NOTIFY) {
        // Process all queued notifications
        std::lock_guard<std::mutex> lock(g_queue_mutex);
        while (!g_queue.empty()) {
            auto& notif = g_queue.front();

            NOTIFYICONDATAW nid = g_nid;
            nid.uFlags = NIF_INFO;
            nid.dwInfoFlags = NIIF_WARNING;
            wcsncpy_s(nid.szInfoTitle, notif.title.c_str(), 63);
            wcsncpy_s(nid.szInfo, notif.body.c_str(), 255);
            Shell_NotifyIconW(NIM_MODIFY, &nid);

            g_queue.pop();
        }
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static void notification_thread_func() {
    // Register window class
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = L"WenzGuardNotify";
    RegisterClassExW(&wc);

    // Create hidden message-only window
    g_msg_hwnd = CreateWindowExW(
        0, L"WenzGuardNotify", L"WenzGuard", 0,
        0, 0, 0, 0, HWND_MESSAGE, NULL, GetModuleHandleW(NULL), NULL
    );

    // Add tray icon
    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = g_msg_hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    g_nid.uCallbackMessage = WM_USER + 1;
    g_nid.hIcon = LoadIconW(NULL, IDI_SHIELD);
    wcscpy_s(g_nid.szTip, L"WenzGuard - Active");
    Shell_NotifyIconW(NIM_ADD, &g_nid);

    // Use NOTIFYICON_VERSION_4 for modern balloon behavior
    g_nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &g_nid);

    // Message pump — runs until WM_QUIT
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Cleanup tray icon
    Shell_NotifyIconW(NIM_DELETE, &g_nid);
}

static void ensure_thread() {
    if (g_thread_started) return;
    g_thread_started = true;

    std::thread t(notification_thread_func);
    t.detach();

    // Wait for window to be created
    for (int i = 0; i < 50 && !g_msg_hwnd; i++) {
        Sleep(100);
    }
}

void show_notification(const std::wstring& title, const std::wstring& message) {
    ensure_thread();

    // Console output (always visible)
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        std::wstring line = L"  >> ALERT: " + title + L" | " + message + L"\n";
        DWORD written;
        WriteConsoleW(hConsole, line.c_str(), (DWORD)line.size(), &written, NULL);
    }

    // Queue for toast notification
    {
        std::lock_guard<std::mutex> lock(g_queue_mutex);
        g_queue.push({ title, message });
    }

    // Wake up the notification thread
    if (g_msg_hwnd) {
        PostMessageW(g_msg_hwnd, WM_WENZGUARD_NOTIFY, 0, 0);
    }
}

void play_alarm_sound() {
    MessageBeep(MB_ICONEXCLAMATION);
}

void cleanup_tray() {
    if (g_msg_hwnd) {
        PostMessageW(g_msg_hwnd, WM_QUIT, 0, 0);
        Sleep(200);
    }
}
