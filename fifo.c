#include "fifo.h"
#include <stdlib.h>
#include <string.h>

static int incWrap(size_t wp, size_t limit) {
    if (++wp == limit)
        wp = 0;
    return wp;
}

int initFifo(Fifo *fifo, size_t fifoSize, size_t typeSize)
{
    if (fifo == NULL) 
        return -1;

    memset(fifo, 0, sizeof(Fifo));
    
    fifo->data = calloc(fifoSize, typeSize);
    if (fifo->data == NULL) {
        return -1;
    }

    if(pthread_mutex_init(&fifo->mutex, NULL)) {
        free(fifo->data);
        fifo->data = NULL;
        return -1;
    }

    fifo->typeSize = typeSize;
    fifo->fifoSize = fifoSize;

    return 0;
}

int pushFifo(Fifo *fifo, void *data)
{
    if (fifo == NULL || data == NULL) 
        return -1;

    int res = 0;
    pthread_mutex_lock(&fifo->mutex);
    size_t next = incWrap(fifo->wp, fifo->fifoSize);
    if (next == fifo->rp) {
        res = -1;
        goto cleanup;
    }

    char *pos = (char *)fifo->data + (fifo->wp * fifo->typeSize);
    memcpy(pos, data, fifo->typeSize);

    fifo->wp = next;

cleanup:
    pthread_mutex_unlock(&fifo->mutex);
    return res;
}

int popFifo(Fifo *fifo, void *out)
{
    if (fifo == NULL || out == NULL)
        return -1;

    int res = 0;
    pthread_mutex_lock(&fifo->mutex);
    if (fifo->rp == fifo->wp) {
        res = -1;
        goto cleanup;
    }

    char *pos = (char *)fifo->data + (fifo->rp * fifo->typeSize);
    memcpy(out, pos, fifo->typeSize);

    fifo->rp = incWrap(fifo->rp, fifo->fifoSize);

cleanup:
    pthread_mutex_unlock(&fifo->mutex);
    return res;
}

void freeFifo(Fifo *fifo)
{
    if (fifo == NULL)
        return;

    if (fifo->data) {
        free(fifo->data);
        fifo->data = NULL;
    }

    pthread_mutex_destroy(&fifo->mutex);
}