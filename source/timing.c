#include <stddef.h>

#ifdef __linux__

#include <sys/time.h>

#endif
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "timing.h"

double timing_get_current_milliseconds() {
#ifdef __linux__
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000 + time.tv_usec / 1000.0;
#elif _WIN32
    FILETIME time;
    GetSystemTimeAsFileTime(&time);
    return ((LONGLONG) time.dwLowDateTime + ((LONGLONG) (time.dwHighDateTime) << 32LL)) / 10000.0;
#else
    return 0;
#endif
}
