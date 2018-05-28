#ifndef APP_H
#define APP_H

#include <stdbool.h>

#include "encoder.h"
#include "grabber.h"
#include "server.h"
#include "window_system.h"

#define APP_MOUSE_SPEED 5.0f
#define APP_FRAME_BUFFER_SIZE (1024*1024)

typedef struct {
    window_system_connection_t *window_system_connection;
    encoder_t *encoder;
    grabber_t *grabber;
    server_t *server;
    int allow_input;

    float mouse_speed;
} app_t;

bool interrupted;

app_t *app_create(window_system_connection_t *window_system_connection, window_t *window,
                  int port, int bit_rate, int out_width, int out_height, int allow_input,
                  grabber_crop_area_t crop);

void app_destroy(app_t *self);

void app_run(app_t *self, int targt_fps);

void app_on_connect(app_t *self, struct lws *socket);

void app_on_close(app_t *self, struct lws *socket);

void app_on_message(app_t *self, struct lws *socket, void *data, size_t len);

#endif
