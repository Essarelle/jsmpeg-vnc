#ifndef PLATFORM_H
#define PLATFORM_H

#include "list.h"

#ifdef __linux__

#include <X11/Xlib.h>

#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#endif

#ifdef _WIN32

char *from_wide(const wchar_t *wide_string);

wchar_t *to_wide(const char *string);

#endif

typedef struct window_t window_t;

window_t *window_new(
    const char *title
#ifdef __linux__
    , Window handle
#endif
#ifdef _WIN32
    , HWND handle
#endif
);

void window_drop(window_t *window);

window_t *window_clone(window_t *window);

const char *window_get_title(window_t *window);

#ifdef __linux__

Window window_get_handle(window_t *window);

#endif

#ifdef _WIN32

HWND window_get_handle(window_t *window);

#endif

typedef struct window_system_connection_t window_system_connection_t;

window_system_connection_t *window_system_connection_new();

void window_system_connection_drop(window_system_connection_t *window_system_connection);

#ifdef __linux__

Display *window_system_connection_get_display(window_system_connection_t *window_system_connection);

#endif

LIST_HEADER(windows_list, windows_list_t, window, window_t)

windows_list_t *windows_list_query(window_system_connection_t *window_system_connection);

window_t *windows_list_find_title_prefix(windows_list_t *windows_list, const char *prefix);

#ifdef __linux__

window_t *windows_list_find_handle(windows_list_t *windows_list, Window handle);

#endif

#ifdef _WIN32

window_t *windows_list_find_handle(windows_list_t *windows_list, HWND handle);

#endif

window_t *windows_list_find_desktop(windows_list_t *windows_list, window_system_connection_t *window_system_connection);

#endif
