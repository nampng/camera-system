#include "fifo.h"
#include <stdlib.h>
#include <string.h>

int incWrap(size_t wp, size_t limit) {
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

    fifo->typeSize = typeSize;
    fifo->fifoSize = fifoSize;
}

int pushFifo(Fifo *fifo, void *data)
{
    if (fifo == NULL)
        return -1;

    size_t next = incWrap(fifo->wp, fifo->fifoSize);
    if (next == fifo->rp) {
        return -1;
    }

    char *pos = (char *)fifo->data + (fifo->wp * fifo->typeSize);
    memcpy(pos, data, fifo->typeSize);

    fifo->wp = next;

    return 0;
}

int popFifo(Fifo *fifo, void *out)
{
    if (fifo == NULL || out == NULL)
        return -1;

    if (fifo->rp == fifo->wp) {
        return -1;
    }

    char *pos = (char *)fifo->data + (fifo->rp * fifo->typeSize);
    memcpy(out, pos, fifo->typeSize);

    fifo->rp = incWrap(fifo->rp, fifo->fifoSize);

    return 0;
}

void freeFifo(Fifo *fifo)
{
    if (fifo == NULL)
        return;

    if (fifo->data) {
        free(fifo->data);
        fifo->data = NULL;
    }
}