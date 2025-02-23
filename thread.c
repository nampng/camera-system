#include "thread.h"
#include "fifo.h"
#include <string.h>
#include <stdlib.h>

int initThreadArg(ThreadArg *arg, Stream *stream, int width, int height, int bpp, size_t numFrames)
{
        memset(arg, 0, sizeof(ThreadArg));

        arg->stream = stream;
        arg->frameSize = width * height * (bpp / 8);
        arg->numFrames = numFrames;
        arg->running = 1;

        if(initFifo(&arg->buffer, numFrames + 1, arg->frameSize)) {
                return -1;
        }

        if(pthread_mutex_init(&arg->mutex, NULL)) {
                freeFifo(&arg->buffer);
                return -1;
        }

        return 0;
}

void freeThreadArg(ThreadArg *arg)
{
        freeFifo(&arg->buffer);
        pthread_mutex_destroy(&arg->mutex);
}
