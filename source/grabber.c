#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#endif

#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#endif

#include "grabber.h"

grabber_t *grabber_create(window_system_connection_t *window_system_connection,
                          window_t *window,
                          grabber_crop_area_t crop) {
    grabber_t *self = (grabber_t *) malloc(sizeof(grabber_t));
    memset(self, 0, sizeof(grabber_t));

    self->window = window;

#ifdef __linux__
    XWindowAttributes window_attributes;
    XGetWindowAttributes(
        window_system_connection_get_display(window_system_connection),
        window_get_handle(window),
        &window_attributes
    );
    self->width = window_attributes.width;
    self->height = window_attributes.height;
#elif WIN32
    RECT rect;
    GetClientRect(window_get_handle(window), &rect);
    self->width = rect.right - rect.left;
    self->height = rect.bottom - rect.top;
#else
    self->width = 100;
    self->height = 100;
#endif

    self->crop = crop;
    if (crop.width == 0 || crop.height == 0) {
        self->crop.width = self->width - crop.x;
        self->crop.height = self->height - crop.y;
    }

    self->width = self->crop.width;
    self->height = self->crop.height;

#ifdef WIN32
    self->windowDC = GetDC(window_get_handle(window));
    self->memoryDC = CreateCompatibleDC(self->windowDC);
    self->bitmap = CreateCompatibleBitmap(self->windowDC, self->width, self->height);

    self->bitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
    self->bitmapInfo.biPlanes = 1;
    self->bitmapInfo.biBitCount = 32;
    self->bitmapInfo.biWidth = self->width;
    self->bitmapInfo.biHeight = -self->height;
    self->bitmapInfo.biCompression = BI_RGB;
    self->bitmapInfo.biSizeImage = 0;
#endif

    self->pixels = malloc(self->width * self->height * 4);

    return self;
}

void grabber_destroy(grabber_t *self) {
    if (self == NULL) { return; }

#ifdef WIN32
    ReleaseDC(window_get_handle(self->window), self->windowDC);
    DeleteDC(self->memoryDC);
    DeleteObject(self->bitmap);
#endif

    free(self->pixels);
    free(self);
}

void *grabber_grab(grabber_t *self, window_system_connection_t *window_system_connection) {
#ifdef __linux__
    XImage *image = XGetImage(
        window_system_connection_get_display(window_system_connection),
        window_get_handle(self->window),
        0,
        0,
        self->width,
        self->height,
        AllPlanes,
        ZPixmap
    );
    memcpy(self->pixels, image->data, self->width * self->height * 4);
    XDestroyImage(image);
#elif WIN32
    SelectObject(self->memoryDC, self->bitmap);
    BitBlt(self->memoryDC, 0, 0, self->width, self->height, self->windowDC, self->crop.x, self->crop.y, SRCCOPY);
    GetDIBits(self->memoryDC, self->bitmap, 0, self->height, self->pixels, (BITMAPINFO *) &(self->bitmapInfo),
              DIB_RGB_COLORS);
#else
    memset(self->pixels, 128, self->width * self->height * 4);
    for (size_t i = 0; i < self->width * self->height * 4; i++) {
        ((char *) self->pixels)[i] = (char) rand();
    }
#endif
    return self->pixels;
}

