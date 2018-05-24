#include <stdlib.h>
#include <stdio.h>
#include "server.h"

static int
callback_websockets(struct lws *wsi, enum lws_callback_reasons reason,
                    void *user, void *in, size_t len);

static struct lws_protocols server_protocols[] = {
    {"http", lws_callback_http_dummy, 0,                              0},
    {"ws",   callback_websockets,     sizeof(struct server_client_t), 1024 * 1024},
    {NULL, NULL,                      0,                              0}
};

static const struct lws_http_mount mount = {
    .mount_next = NULL,
    .mountpoint = "/",
    .origin = "./client",
    .def = "index.html",
    .protocol = NULL,
    .cgienv = NULL,
    .extra_mimetypes = NULL,
    .interpret = NULL,
    .cgi_timeout = 0,
    .cache_max_age = 0,
    .auth_mask = 0,
    .cache_reusable = 0,
    .cache_revalidate = 0,
    .cache_intermediaries = 0,
    .origin_protocol = LWSMPRO_FILE,
    .mountpoint_len = 1,
    .basic_auth_login_file = NULL,
};

server_t *server_create(int port, size_t buffer_size) {
    server_t *self = (server_t *) malloc(sizeof(server_t));
    memset(self, 0, sizeof(server_t));

    self->port = port;
    self->clients = NULL;

//    lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_DEBUG, NULL);
    lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE, NULL);

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(struct lws_context_creation_info));
    info.port = port;
    info.gid = -1;
    info.uid = -1;
    info.user = (void *) self;
    info.protocols = server_protocols;
    info.mounts = &mount;
    self->context = lws_create_context(&info);

    if (!self->context) {
        server_destroy(self);
        return NULL;
    }

    return self;
}

void server_destroy(server_t *self) {
    if (self == NULL) { return; }

    if (self->context) {
        lws_context_destroy(self->context);
    }

    free(self);
}

char *server_get_host_address(server_t *self) {
    char host_name[80];
    struct hostent *host;
    if (gethostname(host_name, sizeof(host_name)) == SOCKET_ERROR || !(host = gethostbyname(host_name))) {
        return "127.0.0.1";
    }

    return inet_ntoa(*(IN_ADDR *) (host->h_addr_list[0]));
}

char *server_get_client_address(server_t *self, struct lws *wsi) {
    static char ip_buffer[32];
    static char name_buffer[32];

    lws_get_peer_addresses(
        wsi, lws_get_socket_fd(wsi),
        name_buffer, sizeof(name_buffer),
        ip_buffer, sizeof(ip_buffer)
    );
    return ip_buffer;
}

void server_update(server_t *self) {
    lws_service(self->context, 0);
}

void server_send(server_t *self, struct lws *socket, void *data, size_t size, enum lws_write_protocol type) {
    lws_start_foreach_llp(struct server_client_t **, client, self->clients)
            {
                if ((*client)->wsi == socket) {
                    size_t send_frames_old_count = (*client)->send_frames_count;
                    (*client)->send_frames_count++;
                    (*client)->send_frames = realloc(
                        (*client)->send_frames,
                        sizeof(struct send_frame_t) * (*client)->send_frames_count
                    );
                    struct send_frame_t *send_frame = &(*client)->send_frames[send_frames_old_count];
                    send_frame->data = malloc(LWS_PRE + size);
                    memcpy((char *) send_frame->data + LWS_PRE, data, size);
                    send_frame->size = size;
                    send_frame->type = type;
                }
            }
    lws_end_foreach_llp(client, next);
    lws_callback_on_writable(socket);
}

void server_broadcast(server_t *self, void *data, size_t size, enum lws_write_protocol type) {
    lws_start_foreach_llp(struct server_client_t **, client, self->clients)
            {
                server_send(self, (*client)->wsi, data, size, type);
            }
    lws_end_foreach_llp(client, next);
}

static int callback_websockets(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    server_t *self = (server_t *) lws_context_user(lws_get_context(wsi));
    struct server_client_t *user_client = (struct server_client_t *) user;

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: {
            lws_ll_fwd_insert(user_client, next, self->clients);
            user_client->wsi = wsi;
            user_client->send_frames = malloc(0);
            user_client->send_frames_count = 0;

            if (self->on_connect) {
                self->on_connect(self, wsi);
            }
            break;
        }

        case LWS_CALLBACK_RECEIVE: {
            if (self->on_message) {
                self->on_message(self, wsi, in, len);
            }
            break;
        }

        case LWS_CALLBACK_SERVER_WRITEABLE: {
            if (user_client->send_frames_count > 0) {
                struct send_frame_t send_frame = user_client->send_frames[0];
                lws_write(wsi, send_frame.data + LWS_PRE, send_frame.size, send_frame.type);
                user_client->send_frames_count--;
            }
            if (user_client->send_frames_count > 0) {
                lws_callback_on_writable(wsi);
            }
            break;
        }

        case LWS_CALLBACK_CLOSED: {
            lws_ll_fwd_remove(struct server_client_t, next, user_client, self->clients);
            for (size_t i = 0; i < user_client->send_frames_count; i++) {
                struct send_frame_t send_frame = user_client->send_frames[i];
                free(send_frame.data);
            }
            free(user_client->send_frames);
            if (self->on_close) {
                self->on_close(self, wsi);
            }
            break;
        }
    }

    return 0;
}
