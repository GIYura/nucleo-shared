#ifndef I2C_H
#define I2C_H

#include <stdbool.h>

#include "stm32f411xe.h"

#include "gpio.h"

typedef enum
{
    I2C_1 = 0,
    I2C_2,
    I2C_3,
    I2C_COUNT
} I2C_NAMES;

typedef struct
{

} I2C_Config_t;

typedef enum
{
    I2C_IDLE = 0,
    I2C_BUSY_TX,
    I2C_BUSY_RX
} I2C_STATE;

typedef struct
{
    I2C_TypeDef* instance;
    I2C_NAMES name;
    Gpio_t sda;
    Gpio_t scl;
    uint8_t devAddress;
    uint8_t* txBuffer;
    uint8_t* rxBuffer;
    uint32_t txLen;
    uint32_t rxLen;
    uint8_t txState;
    uint8_t rxState;
    bool initialized;
} I2C_Handle_t;

void I2C_Init(I2C_Handle_t* const obj, I2C_NAMES name/*, I2C_Config_t* config*/);
void I2C_Deinit(I2C_Handle_t* const obj);

void I2C_Start(I2C_Handle_t* const obj);
void I2C_Stop(I2C_Handle_t* const obj);
void I2C_WriteAddress(I2C_Handle_t* const obj, uint8_t address);
void I2C_WriteData(I2C_Handle_t* const obj, uint8_t data);
void I2C_WaitTrasfetFinished(I2C_Handle_t* const obj);
uint8_t I2C_ReadDataNACK(I2C_Handle_t* const obj);

void I2C_ReadAsync(I2C_Handle_t* const obj, uint8_t devAddr, uint8_t* data, uint8_t size);
void I2C_WriteAsync(I2C_Handle_t* const obj, uint8_t devAddr, uint8_t* data, uint8_t size);

void I2C_MasterTransmit(I2C_Handle_t* const obj, const uint8_t* txBuffer, uint8_t len, uint8_t slaveAddr);
void I2C_MasterReceive(I2C_Handle_t* const obj, uint8_t* rxBuffer, uint8_t len, uint8_t slaveAddr);

#endif /* I2C_H */
