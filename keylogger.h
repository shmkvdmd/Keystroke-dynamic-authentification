#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#include <windows.h>
#include <map>
#include <vector>
#include <chrono>
#include <utility>
#include <fstream>
#include <unordered_map>
#include <QKeySequence>

class KeyLogger {
public:
    KeyLogger();
    ~KeyLogger();
    void Start();
    void Stop();
    void ToggleLogging();
    bool IsLoggingEnabled() const;
    void PrintCurrentMap();
    bool ShouldStop() const;
    static void SetHotkey(const QKeySequence& hotkey);
    static void SetDurationLimit(unsigned int duration_limit);

private:
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static KeyLogger* instance_;
    static std::unordered_map<int, std::string> key_map_;
    std::map<std::pair<int, int>, std::vector<double>> keys_time_;
    std::chrono::steady_clock::time_point press_time_;
    int last_key_;
    HHOOK hook_;
    bool should_stop_;
    bool logging_enabled_ = true;
    bool ctrl_pressed_;
    std::ofstream out_log_file_;
    std::ofstream out_param_file_;
    static QKeySequence hotkey_;
    static unsigned int duration_limit_;

    void LogKeyPressed(int key);
    std::string GetKeyName(int vkCode);
};

#endif // KEYLOGGER_H
