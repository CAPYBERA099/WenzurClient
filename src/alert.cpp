#include "alert.h"
#include <windows.h>

void show_alert(const std::wstring& title, const std::wstring& message) {
    // Показываем MessageBox в отдельном потоке чтобы не блокировать мониторинг
    std::wstring t = title;
    std::wstring m = message;

    MessageBoxW(
        NULL,
        m.c_str(),
        t.c_str(),
        MB_OK | MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND
    );
}

void show_toast(const std::wstring& title, const std::wstring& message) {
    // Простой toast через PowerShell (работает на Windows 10+)
    std::wstring ps_command =
        L"powershell -Command \""
        L"[Windows.UI.Notifications.ToastNotificationManager, Windows.UI.Notifications, ContentType = WindowsRuntime] | Out-Null; "
        L"$xml = [Windows.UI.Notifications.ToastNotificationManager]::GetTemplateContent(1); "
        L"$text = $xml.GetElementsByTagName('text'); "
        L"$text[0].AppendChild($xml.CreateTextNode('" + title + L"')) | Out-Null; "
        L"$text[1].AppendChild($xml.CreateTextNode('" + message + L"')) | Out-Null; "
        L"$toast = [Windows.UI.Notifications.ToastNotification]::new($xml); "
        L"[Windows.UI.Notifications.ToastNotificationManager]::CreateToastNotifier('WenzGuard').Show($toast)"
        L"\"";

    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi;

    std::wstring cmd = ps_command;
    if (CreateProcessW(NULL, &cmd[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

void play_alarm_sound() {
    // Системный звук предупреждения
    MessageBeep(MB_ICONEXCLAMATION);
    // Дополнительный звук
    Beep(1000, 300);
    Beep(1500, 300);
    Beep(1000, 300);
}
