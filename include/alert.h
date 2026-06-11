#pragma once
#include <string>

// Windows Toast notification (balloon via system tray)
void show_notification(const std::wstring& title, const std::wstring& message);

// Sound alert
void play_alarm_sound();

// Cleanup tray icon on exit
void cleanup_tray();
