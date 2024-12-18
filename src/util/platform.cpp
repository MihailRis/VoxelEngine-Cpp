#include "platform.hpp"

#include <time.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#include "stringutil.hpp"
#include "typedefs.hpp"

#ifdef _WIN32
#include <Windows.h>
#pragma comment(lib, "winmm.lib")

void platform::configure_encoding() {
    // set utf-8 encoding to console output
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);
}

std::string platform::detect_locale() {
    LCID lcid = GetThreadLocale();
    wchar_t preferredLocaleName[LOCALE_NAME_MAX_LENGTH];  // locale name format:
                                                          // ll-CC
    if (LCIDToLocaleName(
            lcid, preferredLocaleName, LOCALE_NAME_MAX_LENGTH, 0
        ) == 0) {
        std::cerr
            << "error in platform::detect_locale! LCIDToLocaleName failed."
            << std::endl;
    }
    // ll_CC format
    return util::wstr2str_utf8(preferredLocaleName)
        .replace(2, 1, "_")
        .substr(0, 5);
}

void platform::sleep(size_t millis) {
    // Uses implementation from the SFML library
    // https://github.com/SFML/SFML/blob/master/src/SFML/System/Win32/SleepImpl.cpp

    // Get the minimum supported timer resolution on this system
    static const UINT periodMin = []{
        TIMECAPS tc;
        timeGetDevCaps(&tc, sizeof(TIMECAPS));
        return tc.wPeriodMin;
    }();

    // Set the timer resolution to the minimum for the Sleep call
    timeBeginPeriod(periodMin);

    // Wait...
    Sleep(static_cast<DWORD>(millis));

    // Reset the timer resolution back to the system default
    timeEndPeriod(periodMin);
}

int platform::get_process_id() {
    return GetCurrentProcessId(); 
}

#else // _WIN32

#include <unistd.h>

void platform::configure_encoding() {
}

std::string platform::detect_locale() {
    std::string programLocaleName = setlocale(LC_ALL, nullptr);
    std::string preferredLocaleName =
        setlocale(LC_ALL, "");  // locale name format: ll_CC.encoding
    setlocale(LC_ALL, programLocaleName.c_str());

    return preferredLocaleName.substr(0, 5);
}

void platform::sleep(size_t millis) {
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
}

int platform::get_process_id() {
    return getpid();
}
#endif // _WIN32

void platform::open_folder(const std::filesystem::path& folder) {
    if (!std::filesystem::is_directory(folder)) {
        return;
    }
#ifdef __APPLE__
    auto cmd = "open " + util::quote(folder.u8string());
    system(cmd.c_str());
#elif defined(_WIN32)
    auto cmd = "start explorer " + util::quote(folder.u8string());
    ShellExecuteW(NULL, L"open", folder.wstring().c_str(), NULL, NULL, SW_SHOWDEFAULT);
#else
    auto cmd = "xdg-open " + util::quote(folder.u8string());
    system(cmd.c_str());
#endif
}
