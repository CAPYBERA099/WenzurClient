#pragma once
#include <string>

// Показать Windows уведомление (MessageBox)
void show_alert(const std::wstring& title, const std::wstring& message);

// Показать toast notification (Windows 10+)
void show_toast(const std::wstring& title, const std::wstring& message);

// Звуковой сигнал тревоги
void play_alarm_sound();
