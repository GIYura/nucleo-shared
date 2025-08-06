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
    bool overwrite;
} Buffer_t;

/*Brief: Create circular buffer
 * [in] - buffer - pointer to buffer object
 * [in] - data - storage
 * [in] - length - size of storage
 * [in] - typeSize - size of stored item
 * [in] - overwrite - true - replace old data with new; false - discard new data
 * [out] - none
 * */
void BufferCreate(Buffer_t* const buffer, void* const data, uint16_t length, uint16_t typeSize, bool overwrite);

/*Brief: Calculate circular buffer capacity in bytes
 * [in] - buffer - pointer to buffer object
 * [out] - capacity
 * */
uint16_t BufferCapacity(const Buffer_t* const buffer);

/*Brief: Clear circular buffer capacity
 * [in] - buffer - pointer to buffer object
 * [out] - none
 * */
void BufferClear(Buffer_t* const buffer);

/*Brief: Calculate number of items in circular buffer
 * [in] - buffer - pointer to buffer object
 * [out] - number of item in the buffer
 * */
uint16_t BufferCount(const Buffer_t* const buffer);

/*Brief: Store data in circular buffer
 * [in] - buffer - pointer to buffer object
 * [in] - data - pointer to data to store
 * [in] - size - data size
 * [out] - none
 * */
bool BufferPut(Buffer_t* const buffer, const void* const data, uint16_t size);

/*Brief: Retrieve data from circular buffer
 * [in] - buffer - pointer to buffer object
 * [in] - data - pointer to data to store
 * [in] - size - data size
 * [out] - none
 * */
bool BufferGet(Buffer_t* const buffer, void* data, uint16_t size);

#endif /* BUFFER_H */
