#pragma once
#include <string>

// Windows Toast notification (balloon)
void show_notification(const std::wstring& title, const std::wstring& message);

// Sound alert
void play_alarm_sound();
