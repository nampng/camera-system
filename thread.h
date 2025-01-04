#include <pthread.h>
#include <stdint.h>

// fwd
typedef struct _Stream Stream;

typedef struct _ThreadArg {
        Stream *stream;
        uint8_t *buffer;
        int running;
        size_t bufferSize;
        pthread_mutex_t mutex;
} ThreadArg;

int initThreadArg(ThreadArg *arg, Stream *stream, int width, int height, int bpp);
void freeThreadArg(ThreadArg *arg);
