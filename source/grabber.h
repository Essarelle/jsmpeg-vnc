#ifndef GRABBER_H
#define GRABBER_H

#include "window_system.h"

typedef struct {
    int x, y, width, height;
} grabber_crop_area_t;

typedef struct {
    window_t *window;
#ifdef WIN32
    HDC windowDC;
    HDC memoryDC;
    HBITMAP bitmap;
    BITMAPINFOHEADER bitmapInfo;
#endif

    int width;
    int height;

    void *pixels;
    grabber_crop_area_t crop;
} grabber_t;

grabber_t *grabber_create(window_system_connection_t *window_system_connection,
                          window_t *window,
                          grabber_crop_area_t crop);

void grabber_destroy(grabber_t *self);

void *grabber_grab(grabber_t *self, window_system_connection_t *window_system_connection);

#endif
