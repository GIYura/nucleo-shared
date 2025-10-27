#ifndef ADXL345_H
#define ADXL345_H

#include <stdint.h>
#include <stdbool.h>

#define ADXL345_ID              0xE5

typedef void (*ADXL_RequestHandler_t)(void* value, void* context);

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} Acceleration_t;

/*Brief: ADXL345 initialization over SPI
 * [in] - none
 * [out] - none
 * */
void ADXL_InitSPI(void);

/*Brief: ADXL345 read single register async
 * [in] - address - register address
 * [in] - callback - callback function on receive done
 * [in] - context - response storage address
 * [out] - none
 * */
void ADXL_ReadRegisterAsyncSPI(uint8_t address, ADXL_RequestHandler_t callback, void* context);

/*Brief: ADXL345 write single register async
 * [in] - address - register address
 * [in] - value - new value
 * [out] - none
 * */
void ADXL_WriteRegisterAsyncSPI(uint8_t address, ADXL_RequestHandler_t callback, void* value);

/*Brief: ADXL345 read vector async
 * [in] - address - register address
 * [in] - callback - callback function on receive done
 * [in] - context - response storage address
 * [out] - none
 * */
void ADXL_ReadVectorAsyncSPI(uint8_t address, ADXL_RequestHandler_t callback, void* context);

/*Brief: ADXL345 initialization over I2C
 * [in] - none
 * [out] - none
 * */
void ADXL_InitI2C(void);

/*Brief: ADXL345 read single register async
 * [in] - address - register address
 * [in] - callback - callback function on receive done
 * [in] - context - response storage address
 * [out] - none
 * */
void ADXL_ReadRegisterAsyncI2C(uint8_t address, ADXL_RequestHandler_t callback, void* context);

/*Brief: ADXL345 write single register async
 * [in] - address - register address
 * [in] - value - new value
 * [out] - none
 * */
void ADXL_WriteRegisterAsyncI2C(uint8_t address, ADXL_RequestHandler_t callback, void* value);

/*Brief: ADXL345 read vector async
 * [in] - address - register address
 * [in] - callback - callback function on receive done
 * [in] - context - response storage address
 * [out] - none
 * */
void ADXL_ReadVectorAsyncI2C(uint8_t address, ADXL_RequestHandler_t callback, void* context);

#endif /* ADXL345_H */
