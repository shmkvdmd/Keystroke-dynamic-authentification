#include "screentaker.h"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <iomanip>

using namespace Gdiplus;

ScreenTaker::ScreenTaker() : running_(false) {
    GdiplusStartupInput gdipStartupInput;
    GdiplusStartup(&gdip_token_, &gdipStartupInput, NULL);
}

ScreenTaker::~ScreenTaker() {
    StopTakingScreenshots();
    GdiplusShutdown(gdip_token_);
}

void ScreenTaker::StartTakingScreenshots(const std::wstring& directory) {
    StopTakingScreenshots();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        directory_ = directory;
        running_ = true;
    }

    screenshot_thread_ = std::thread(&ScreenTaker::ScreenshotThreadFunction, this);
}

void ScreenTaker::StopTakingScreenshots() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();

    if (screenshot_thread_.joinable()) {
        screenshot_thread_.join();
    }
}

void ScreenTaker::SetInterval(int interval_ms) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        interval_ms_ = interval_ms;
    }
    cv_.notify_all();
}

void ScreenTaker::ScreenshotThreadFunction() {
    std::unique_lock<std::mutex> lock(mutex_);

    while (running_) {
        lock.unlock();
        TakeScreenshot();
        lock.lock();

        cv_.wait_for(lock, std::chrono::milliseconds(interval_ms_), [this] { return !running_; });
    }
}

void ScreenTaker::TakeScreenshot() {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen);

    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
    HGDIOBJ old_obj = SelectObject(hdcMemDC, hbmScreen);

    BitBlt(hdcMemDC, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);

    Gdiplus::Bitmap bmp(hbmScreen, NULL);
    CLSID pngClsid;
    if (GetEncoderClsid(L"image/png", &pngClsid) == -1) {
        std::cerr << "Failed to get PNG encoder CLSID." << std::endl;
    } else {
        std::wstringstream filename;
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        filename << directory_ << L"/screen_" << std::put_time(std::localtime(&time_t_now), L"%Y%m%d_%H%M%S") << L".png";

        bmp.Save(filename.str().c_str(), &pngClsid, NULL);
    }

    SelectObject(hdcMemDC, old_obj);
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
}

int ScreenTaker::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;
    UINT size = 0;

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;

    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    free(pImageCodecInfo);
    return -1;
}
