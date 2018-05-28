#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include "app.h"

void exit_usage(char *self_name) {
    printf(
        "Usage: %s [options] <window name>\n\n"

        "Options:\n"
        "  -b bitrate in kilobit/s (default: estimated by output size)\n"
        "  -s output size as WxH. E.g: -s 640x480 (default: same as window size)\n"
        "  -f target framerate (default: 60)\n"
        "  -p port (default: 8080)\n"
        "  -c crop area in the captured window as X,Y,W,H. E.g.: -c 200,300,640,480\n"
        "  -i enable/disable remote input. E.g. -i 0 (default: 1)\n\n"

        "Use \"desktop\" as the window name to capture the whole Desktop. Use \"cursor\"\n"
        "to capture the window at the current cursor position.\n\n"

        "To enable mouse lock in the browser (useful for games that require relative\n"
        "mouse movements, not absolute ones), append \"?mouselock\" at the target URL.\n"
        "i.e: http://<server-ip>:8080/?mouselock\n\n",
        self_name
    );
    exit(0);
}

void sigint_handler(int sig) {
    interrupted = true;
}

#ifndef _WIN32

int main(int argc, char *argv[]) {
#else
    int wmain(int argc, wchar_t *argv_wide[]) {
        char **argv = malloc(sizeof(char *) * argc);
        for (size_t i = 0; i < argc; i++) {
            argv[i] = from_wide(argv_wide[i]);
        }
#endif
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
    if (argc < 2) {
        exit_usage(argv[0]);
    }

    int bit_rate = 0,
        fps = 60,
        port = 8080,
        width = 0,
        height = 0,
        allow_input = 1;

    grabber_crop_area_t crop = {0, 0, 0, 0};

    // Parse command line options
    for (int i = 1; i < argc - 1; i += 2) {
        if (strlen(argv[i]) < 2 || i >= argc - 2 || argv[i][0] != '-') {
            exit_usage(argv[0]);
        }

        switch (argv[i][1]) {
            case 'b':
                bit_rate = strtol(argv[i + 1], NULL, 0) * 1000;
                break;
            case 'p':
                port = strtol(argv[i + 1], NULL, 0);
                break;
            case 's':
                sscanf(argv[i + 1], "%dx%d", &width, &height);
                break;
            case 'f':
                fps = strtol(argv[i + 1], NULL, 0);
                break;
            case 'i':
                allow_input = strtol(argv[i + 1], NULL, 0);
                break;
            case 'c':
                sscanf(argv[i + 1], "%d,%d,%d,%d", &crop.x, &crop.y, &crop.width, &crop.height);
                break;
            default:
                exit_usage(argv[0]);
        }
    }

    // Find target window
    char *window_title = argv[argc - 1];
    window_system_connection_t *window_system_connection = window_system_connection_new();
    windows_list_t *windows_list = windows_list_query(window_system_connection);
    printf("Windows:\n");
    for (size_t i = 0; i < windows_list_get_length(windows_list); i++) {
        printf("%lu. %s\n", (unsigned long) i, window_get_title(windows_list_get(windows_list, i)));
    }
    window_t *window = NULL;
    if (strcmp(window_title, "desktop") == 0) {
        window = windows_list_find_desktop(windows_list, window_system_connection);
    } else {
        window = windows_list_find_title_prefix(windows_list, window_title);
    }
//    else if (strcmp(window_title, "cursor") == 0) {
//       POINT cursor;
//       GetCursorPos(&cursor);
//       window = WindowFromPoint(cursor);
//    }

    if (window == NULL) {
        printf("No window with title starting with \"%s\"\n", window_title);
        return 0;
    }

    // Start the app
    app_t *app = app_create(window_system_connection, window, port, bit_rate, width, height, allow_input, crop);

    if (!app) {
        return 1;
    }

    printf(
        "Window"
        #ifdef __linux__
        " 0x%lu"
        #endif
        #ifdef _WIN32
        " 0x%p"
        #endif
        ": \"%s\"\n"
        "Window size: %dx%d, output size: %dx%d, bit rate: %lli kb/s\n\n"
        "Server started on: http://%s:%d/\n\n",
#ifdef __linux__
        window_get_handle(window),
#endif
#ifdef _WIN32
        window_get_handle(window),
#endif
        window_get_title(window),
        app->grabber->width,
        app->grabber->height,
        app->encoder->out_width,
        app->encoder->out_height,
        app->encoder->context->bit_rate / 1000,
        server_get_host_address(app->server),
        app->server->port
    );

    app_run(app, fps);

    app_destroy(app);

#ifdef _WIN32
    for (size_t i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
#endif
    windows_list_drop(windows_list);
    window_system_connection_drop(window_system_connection);

    return 0;
}
