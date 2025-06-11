#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint16_t start;
    uint16_t end;
    uint8_t* data;
    uint16_t length;
    uint16_t typeSize;
    bool overflow;
} Buffer_t;

void BufferCreate(Buffer_t* buff);

#endif /* BUFFER_H */

