#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "buffer.h"

static uint16_t NextIndex(const Buffer_t* const buffer, uint16_t index)
{
    return (index + buffer->typeSize) % buffer->length;
}

static bool BufferIsFull(const Buffer_t* const buffer)
{
    return NextIndex(buffer, buffer->end) == buffer->start;
}

static bool BufferIsEmpty(const Buffer_t* const buffer)
{
    return buffer->end == buffer->start;
}

void BufferCreate(Buffer_t* const buffer, void* const data, uint16_t length, uint16_t typeSize, bool overwrite)
{
    assert(buffer != NULL);
    assert(data != NULL);

    buffer->data = data;
    buffer->end = 0;
    buffer->length = length;
    buffer->overwrite = overwrite;
    buffer->start = 0;
    buffer->typeSize = typeSize;
}

uint16_t BufferCapacity(const Buffer_t* const buffer)
{
    return (buffer->length - buffer->typeSize) / buffer->typeSize;
}

void BufferClear(Buffer_t* const buffer)
{
    buffer->end = 0;
    buffer->start = 0;
}

uint16_t BufferCount(const Buffer_t* const buffer)
{
    uint16_t count;

    if (buffer->end >= buffer->start)
    {
        count = buffer->end - buffer->start;
    }
    else
    {
        count = buffer->length - buffer->start + buffer->end;
    }

    return (count / buffer->typeSize);
}

bool BufferPut(Buffer_t* const buffer, const void* const data, uint16_t size)
{
    assert(buffer->data != NULL && data != NULL);

    if (BufferIsFull(buffer))
    {
        if (!buffer->overwrite)
        {
            return false;
        }

        buffer->start = NextIndex(buffer, buffer->start);
    }
    memcpy(&buffer->data[buffer->end], data, buffer->typeSize);
    buffer->end = NextIndex(buffer, buffer->end);

    return true;
}

bool BufferGet(Buffer_t* const buffer, void* data, uint16_t size)
{
    assert(buffer->data != NULL);
    assert(data != NULL);
    assert(size == buffer->typeSize);

    if (BufferIsEmpty(buffer))
    {
        return false;
    }

    memcpy(data, &(buffer->data[buffer->start]), buffer->typeSize);
    buffer->start = NextIndex(buffer, buffer->start);

    return true;
}
