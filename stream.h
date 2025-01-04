#ifndef STREAM_H
#define STREAM_H

#include <gst/gst.h>

typedef struct _Stream {
        GstElement *pipeline;
        GstElement *source, *raw, *sink;
} Stream;

int initStream(Stream *stream);
void freeStream(Stream *stream);

void *startStream(void *arg);

#endif // STREAM_H
