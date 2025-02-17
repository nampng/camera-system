#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <stddef.h>

typedef struct _Fifo {
        void *data;
        size_t typeSize;
        size_t wp;
        size_t rp;
        size_t fifoSize;
} Fifo;

int initFifo(Fifo *fifo, size_t fifoSize, size_t typeSize);
int pushFifo(Fifo *fifo, void *data);
int popFifo(Fifo *fifo, void *out);
void freeFifo(Fifo *fifo);

#endif // FIFO_H
