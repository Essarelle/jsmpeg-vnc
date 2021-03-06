#ifndef ENCODER_H
#define ENCODER_H

#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"

typedef struct {
    AVCodec *codec;
    AVCodecContext *context;
    AVFrame *frame;
    void *frame_buffer;

    int in_width, in_height;
    int out_width, out_height;

    AVPacket packet;
    struct SwsContext *sws;
} encoder_t;


encoder_t *encoder_create(int in_width, int in_height, int out_width, int out_height, int bitrate);

void encoder_destroy(encoder_t *self);

void encoder_encode(encoder_t *self, void *rgb_pixels, void *encoded_data, size_t *encoded_size);

#endif
