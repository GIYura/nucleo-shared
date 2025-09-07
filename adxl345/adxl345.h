#ifndef ADXL345_H
#define ADXL345_H

#include <stdint.h>

typedef void (*ADXL_RequestHandler_t)(void* value, void* context);

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} Acceleration_t;

/*Brief: ADXL345 initialization
 * [in] - none
 * [out] - none
 * */
void ADXL_Init(void);

/*Brief: ADXL345 read single register async
 * [in] - address - register address
 * [in] - callback - callback function on receive done
 * [in] - context - response storage address
 * [out] - none
 * */
void ADXL_ReadRegisterAsync(uint8_t address, ADXL_RequestHandler_t callback, void* context);

/*Brief: ADXL345 write single register async
 * [in] - address - register address
 * [in] - value - new value
 * [out] - none
 * */
void ADXL_WriteRegisterAsync(uint8_t address, void* value);

/*Brief: ADXL345 read vector async
 * [in] - address - register address
 * [in] - callback - callback function on receive done
 * [in] - context - response storage address
 * [out] - none
 * */
void ADXL_ReadVectorAsync(uint8_t address, ADXL_RequestHandler_t callback, void* context);

#endif /* ADXL345_H */
