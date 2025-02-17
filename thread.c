#include "thread.h"
#include <string.h>
#include <stdlib.h>

int initThreadArg(ThreadArg *arg, Stream *stream, int width, int height, int bpp)
{
        memset(arg, 0, sizeof(ThreadArg));

        arg->stream = stream;
        arg->bufferSize = width * height * (bpp / 8);
        arg->buffer = (uint8_t *)calloc(arg->bufferSize, sizeof(uint8_t));
        arg->running = 1;
        if(pthread_mutex_init(&arg->mutex, NULL)) {
                free(arg->buffer);
                arg->buffer = NULL;
                return -1;
        }

        return 0;
}

void freeThreadArg(ThreadArg *arg)
{
        if (arg->buffer)
                free(arg->buffer);

        pthread_mutex_destroy(&arg->mutex);
}
