#ifndef SCREENTAKER_H
#define SCREENTAKER_H

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

class ScreenTaker {
public:
    ScreenTaker();
    ~ScreenTaker();

    void StartTakingScreenshots(const std::wstring& directory);
    void StopTakingScreenshots();
    void SetInterval(int interval_ms);
private:
    void ScreenshotThreadFunction();
    void TakeScreenshot();
    int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

    std::thread screenshot_thread_;
    std::atomic<bool> running_;
    ULONG_PTR gdip_token_;

    std::mutex mutex_;
    std::condition_variable cv_;

    int interval_ms_ = 5000;

    std::wstring directory_;
};

#endif // SCREENTAKER_H
