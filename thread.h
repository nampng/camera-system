#ifndef THREAD_H
#define THREAD_H

#include "fifo.h"

#include <pthread.h>
#include <stdint.h>

// fwd
typedef struct _Stream Stream;

typedef struct _ThreadArg {
        Stream *stream;
        Fifo buffer;
        int running;
        size_t numFrames;
        size_t frameSize;
        pthread_mutex_t mutex;
} ThreadArg;

int initThreadArg(ThreadArg *arg, Stream *stream, int width, int height, int bpp, size_t numFrames);
void freeThreadArg(ThreadArg *arg);

#endif // THREAD_H