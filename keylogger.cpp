#include "keylogger.h"
#include <iostream>
#include <iomanip>
#include <QDebug>
#include <ctime>

KeyLogger* KeyLogger::instance_ = nullptr;
QKeySequence KeyLogger::hotkey_;
unsigned int KeyLogger::duration_limit_;

std::unordered_map<int, std::string> KeyLogger::key_map_ = {
    {VK_BACK, "Backspace"},
    {VK_TAB, "Tab"},
    {VK_RETURN, "Enter"},
    {VK_SHIFT, "Shift"},
    {VK_CONTROL, "Ctrl"},
    {VK_MENU, "Alt"},
    {VK_PAUSE, "Pause"},
    {VK_CAPITAL, "Caps Lock"},
    {VK_ESCAPE, "Escape"},
    {VK_SPACE, "Space"},
    {VK_PRIOR, "Page Up"},
    {VK_NEXT, "Page Down"},
    {VK_END, "End"},
    {VK_HOME, "Home"},
    {VK_LEFT, "Left Arrow"},
    {VK_UP, "Up Arrow"},
    {VK_RIGHT, "Right Arrow"},
    {VK_DOWN, "Down Arrow"},
    {VK_INSERT, "Insert"},
    {VK_DELETE, "Delete"},
    {VK_LWIN, "Left Windows"},
    {VK_RWIN, "Right Windows"},
    {VK_APPS, "Applications"},
    {VK_NUMLOCK, "Num Lock"},
    {VK_SCROLL, "Scroll Lock"},
    {VK_F1, "F1"}, {VK_F2, "F2"}, {VK_F3, "F3"}, {VK_F4, "F4"},
    {VK_F5, "F5"}, {VK_F6, "F6"}, {VK_F7, "F7"}, {VK_F8, "F8"},
    {VK_F9, "F9"}, {VK_F10, "F10"}, {VK_F11, "F11"}, {VK_F12, "F12"},
    {VK_LSHIFT, "Left Shift"}, {VK_RSHIFT, "Right Shift"},
    {VK_LCONTROL, "Left Ctrl"}, {VK_RCONTROL, "Right Ctrl"},
    {VK_LMENU, "Left Alt"}, {VK_RMENU, "Right Alt"},
    {VK_OEM_1, ";"},
    {VK_OEM_PLUS, "="},
    {VK_OEM_COMMA, ","},
    {VK_OEM_MINUS, "-"},
    {VK_OEM_PERIOD, "."},
    {VK_OEM_2, "/"},
    {VK_OEM_3, "`"},
    {VK_OEM_4, "["},
    {VK_OEM_5, "\\"},
    {VK_OEM_6, "]"},
    {VK_OEM_7, "'"},
    {VK_OEM_102, "\\"}
};

KeyLogger::KeyLogger()
    : last_key_(0), hook_(nullptr), should_stop_(false), ctrl_pressed_(false), logging_enabled_(true),
    out_log_file_("../logs/key_logs.txt", std::ios_base::app),
    out_param_file_("../logs/param_logs.txt", std::ios_base::app) {
    instance_ = this;
}

KeyLogger::~KeyLogger() {
    Stop();
    PrintCurrentMap();
    if (out_log_file_.is_open()) {
        out_log_file_.close();
    }
    if (out_param_file_.is_open()) {
        out_param_file_.close();
    }
}

void KeyLogger::ToggleLogging() {
    logging_enabled_ = !logging_enabled_;
}

bool KeyLogger::IsLoggingEnabled() const {
    return logging_enabled_;
}

void KeyLogger::Start() {
    hook_ = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    if (!hook_) {
        std::cerr << "Failed to set hook!" << std::endl;
    }
}

void KeyLogger::Stop() {
    if (hook_) {
        UnhookWindowsHookEx(hook_);
        hook_ = nullptr;
    }
}

void KeyLogger::SetHotkey(const QKeySequence& hotkey) {
    hotkey_ = hotkey;
}

void KeyLogger::SetDurationLimit(unsigned int duration_limit) {
    duration_limit_ = duration_limit;
}

LRESULT CALLBACK KeyLogger::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        int vkCode = static_cast<int>(kbdStruct->vkCode);

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            // Проверяем горячую клавишу для включения/выключения логирования
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
                Qt::Key qtKey = static_cast<Qt::Key>(vkCode);
                QKeySequence currentSequence(Qt::CTRL | qtKey);
                if (currentSequence == hotkey_) {
                    // Используем instance_ для вызова нестатического метода ToggleLogging
                    if (instance_) {
                        instance_->ToggleLogging();  // Включаем/выключаем логирование
                        qDebug() << "Логирование переключено. Состояние:" << instance_->logging_enabled_;
                    }
                }
            }

            // Записываем нажатие клавиши, если логирование включено
            if (instance_ && instance_->IsLoggingEnabled()) {
                instance_->LogKeyPressed(vkCode);
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}




void KeyLogger::LogKeyPressed(int key) {
    if (!logging_enabled_) {
        return;
    }

    auto now = std::chrono::steady_clock::now();

    if (last_key_ != 0) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - press_time_).count();
        auto current_time = std::chrono::system_clock::now();
        std::time_t current_time_c = std::chrono::system_clock::to_time_t(current_time);
        out_log_file_ << std::put_time(std::localtime(&current_time_c), "%Y-%m-%d %H:%M:%S") << " Key pressed " << GetKeyName(key) << "\n";

        if (duration < duration_limit_) {
            std::pair<int, int> ordered_pair = (last_key_ < key) ? std::make_pair(last_key_, key) : std::make_pair(key, last_key_);
            keys_time_[ordered_pair].push_back(static_cast<double>(duration));
        }
    }

    press_time_ = now;
    last_key_ = key;
}


void KeyLogger::PrintCurrentMap() {
    auto current_time = std::chrono::system_clock::now();
    std::time_t current_time_c = std::chrono::system_clock::to_time_t(current_time);

    out_param_file_ << "\n" << std::put_time(std::localtime(&current_time_c), "%Y-%m-%d %H:%M:%S") << "\n";

    for (const auto& [key, values] : keys_time_) {
        out_param_file_ << GetKeyName(key.first) << "  AND  " << GetKeyName(key.second) << " ";
        for (const auto& value : values) {
            out_param_file_ << value << "ms ";
        }
        out_param_file_ << "\n";
    }
}

bool KeyLogger::ShouldStop() const {
    return should_stop_;
}

std::string KeyLogger::GetKeyName(int vkCode) {
    if ((vkCode >= 0x30 && vkCode <= 0x39) || (vkCode >= 0x41 && vkCode <= 0x5A)) {
        return std::string(1, static_cast<char>(vkCode));
    }
    if (key_map_.find(vkCode) != key_map_.end()) {
        return key_map_[vkCode];
    }
    return "Unknown";
}

