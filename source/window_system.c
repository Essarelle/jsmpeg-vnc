#include <malloc.h>
#include <string.h>
#include "window_system.h"

#ifdef _WIN32

char *from_wide(const wchar_t *wide_string) {
    size_t length = wcslen(wide_string);
    size_t wide_chars_count = (size_t) WideCharToMultiByte(CP_UTF8, 0, wide_string, length, NULL, 0, NULL, NULL);
    if (wide_chars_count == 0) {
        return "";
    };
    char *string = malloc((wide_chars_count + 1) * sizeof(char));
    WideCharToMultiByte(CP_UTF8, 0, wide_string, length, string, wide_chars_count, NULL, NULL);
    string[wide_chars_count] = '\0';
    return string;
}

wchar_t *to_wide(const char *string) {
    size_t length = strlen(string);
    size_t chars_count = (size_t) MultiByteToWideChar(CP_UTF8, 0, string, length, 0, 0);
    if (chars_count == 0) {
        return (wchar_t *) L"";
    };
    wchar_t *wide_string = malloc((chars_count + 1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, string, length, wide_string, chars_count);
    wide_string[chars_count] = L'\0';
    return wide_string;
}

#endif

struct window_t {
    char *title;
#ifdef __linux__
    Window handle;
#endif
#ifdef _WIN32
    HWND handle;
#endif
};

window_t *window_new(
    const char *title
#ifdef __linux__
    , Window handle
#endif
#ifdef _WIN32
    , HWND handle
#endif
) {
    window_t *window = malloc(sizeof(window_t));
    window->title = strdup(title);
#ifdef __linux__
    window->handle = handle;
#endif
#ifdef _WIN32
    window->handle = handle;
#endif
    return window;
}

void window_drop(window_t *window) {
    free(window->title);
    free(window);
}

window_t *window_clone(window_t *window) {
    return window_new(
        window->title
#ifdef __linux__
        , window->handle
#endif
#ifdef _WIN32
        , window->handle
#endif
    );
}

const char *window_get_title(window_t *window) {
    return window->title;
}

#ifdef __linux__

Window window_get_handle(window_t *window) {
    return window->handle;
}

#endif

#ifdef _WIN32
HWND window_get_handle(window_t *window) {
    return window->handle;
}
#endif

struct window_system_connection_t {
#ifdef __linux__
    Display *display;
#endif
};

window_system_connection_t *window_system_connection_new() {
    window_system_connection_t *window_system_connection = malloc(sizeof(window_system_connection_t));
#ifdef __linux__
    window_system_connection->display = XOpenDisplay(NULL);
#endif
    return window_system_connection;
}

void window_system_connection_drop(window_system_connection_t *window_system_connection) {
#ifdef __linux__
    XCloseDisplay(window_system_connection->display);
#endif
    free(window_system_connection);
}

#ifdef __linux__

Display *window_system_connection_get_display(window_system_connection_t *window_system_connection) {
    return window_system_connection->display;
}

#endif

LIST_SOURCE(windows_list, windows_list_t, window, window_t)

#ifdef __linux__

void window_from_name_search(window_system_connection_t *window_system_connection,
                             windows_list_t *windows_list,
                             Window handle) {
    char *title = NULL;
    if (XFetchName(window_system_connection_get_display(window_system_connection), handle, &title) != 0) {
        if (strlen(title) > 0) {
            window_t *window = window_new(title, handle);
            windows_list_insert(windows_list, window);
            free(window);
        }
        XFree(title);
    }
    Window root;
    Window parent;
    Window *children;
    unsigned int children_count;
    if (XQueryTree(
        window_system_connection_get_display(window_system_connection),
        handle,
        &root,
        &parent,
        &children,
        &children_count
    ) != 0) {
        for (unsigned int i = 0; i < children_count; i++) {
            window_from_name_search(window_system_connection, windows_list, children[i]);
        }
        XFree(children);
    }
}

#endif

#ifdef _WIN32

BOOL CALLBACK windows_list_query_callback(HWND handle, LPARAM lParam) {
    if (IsWindowVisible(handle)) {
        windows_list_t *windows_list = (windows_list_t *) lParam;
        int title_length = GetWindowTextLengthW(handle);
        wchar_t *wide_title = malloc(sizeof(wchar_t) * (title_length + 1));
        GetWindowTextW(handle, wide_title, title_length + 1);
        char *title = from_wide(wide_title);
        free(wide_title);
        window_t *window = window_new(title, handle);
        windows_list_insert(windows_list, window);
        free(window);
        free(title);
    }
    return TRUE;
}

#endif

windows_list_t *windows_list_query(window_system_connection_t *window_system_connection) {
#ifdef __linux__
    windows_list_t *windows_list = windows_list_new();
    window_t *desktop_window = window_new(
        "Desktop",
        DefaultRootWindow(window_system_connection_get_display(window_system_connection))
    );
    windows_list_insert(windows_list, desktop_window);
    window_drop(desktop_window);
    window_from_name_search(
        window_system_connection,
        windows_list,
        DefaultRootWindow(window_system_connection_get_display(window_system_connection))
    );
    return windows_list;
#elif _WIN32
    windows_list_t *windows_list = windows_list_new();
    window_t *desktop_window = window_new("Desktop", GetDesktopWindow());
    windows_list_insert(windows_list, desktop_window);
    window_drop(desktop_window);
    EnumWindows(windows_list_query_callback, (LPARAM) windows_list);
    return windows_list;
#else
#error
    return NULL;
#endif
}

window_t *windows_list_find_title_prefix(windows_list_t *windows_list, const char *prefix) {
    for (size_t i = 0; i < windows_list_get_length(windows_list); i++) {
        window_t *window = windows_list_get(windows_list, i);
        if (strncmp(window_get_title(window), prefix, strlen(prefix)) == 0) {
            return window;
        }
    }
    return NULL;
}

#ifdef __linux__

window_t *windows_list_find_handle(windows_list_t *windows_list, Window handle) {
    for (size_t i = 0; i < windows_list_get_length(windows_list); i++) {
        window_t *window = windows_list_get(windows_list, i);
        if (window_get_handle(window) == handle) {
            return window;
        }
    }
    return NULL;
}

#endif

#ifdef _WIN32

window_t *windows_list_find_handle(windows_list_t *windows_list, HWND handle) {
    for (size_t i = 0; i < windows_list_get_length(windows_list); i++) {
        window_t *window = windows_list_get(windows_list, i);
        if (window_get_handle(window) == handle) {
            return window;
        }
    }
    return NULL;
}

#endif

window_t *
windows_list_find_desktop(windows_list_t *windows_list, window_system_connection_t *window_system_connection) {
#ifdef __linux__
    return windows_list_find_handle(
        windows_list,
        DefaultRootWindow(window_system_connection_get_display(window_system_connection))
    );
#elif _WIN32
    return windows_list_find_handle(windows_list, GetDesktopWindow());
#else
#error
    return NULL;
#endif
}
