#ifndef SERVER_H
#define SERVER_H

#include "libwebsockets.h"

struct send_frame_t {
    void *data;
    size_t size;
    enum lws_write_protocol type;
};

struct server_client_t {
    struct lws *wsi;
    struct send_frame_t *send_frames;
    size_t send_frames_count;
    struct server_client_t *next;
};

typedef struct server_t {
    struct lws_context *context;
    void *user;

    int port;
    struct server_client_t *clients;

    void (*on_connect)(struct server_t *server, struct lws *wsi);

    void (*on_message)(struct server_t *server, struct lws *wsi, void *in, size_t len);

    void (*on_close)(struct server_t *server, struct lws *wsi);
} server_t;


server_t *server_create(int port, size_t buffer_size);

void server_destroy(server_t *self);

char *server_get_host_address(server_t *self);

char *server_get_client_address(server_t *self, struct lws *wsi);

void server_update(server_t *self);

void server_send(server_t *self, struct lws *socket, void *data, size_t size, enum lws_write_protocol type);

void server_broadcast(server_t *self, void *data, size_t size, enum lws_write_protocol type);

#endif
