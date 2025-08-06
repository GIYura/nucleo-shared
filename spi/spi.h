#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stdbool.h>

#include "stm32f411xe.h"

typedef enum
{
    SPI_1 = 0,
    SPI_2,
    SPI_3,
    SPI_4,
    SPI_5,
    SPI_COUNT
} SPI_NAMES;

typedef enum
{
    CPOL_0 = 0,
    CPOL_1
} SPI_POLARITY;

typedef enum
{
    CPHA_0 = 0,
    CPHA_1
} SPI_PHASE;

typedef struct
{
    SPI_TypeDef* instance;
    SPI_NAMES name;
    bool initialized;
} Spi_t;

/*Brief: SPI initialization
 * [in] - obj - pointer to SPI object
 * [in] - name - SPI name
 * [in] - polarity - SPI clock polarity
 * [in] - phase - SPI clock phase
 * [in] - frequencyHz - SPI frequency in Hz
 * [out] - none
 * */
void SpiInit(Spi_t* const obj, SPI_NAMES name, SPI_POLARITY polarity, SPI_PHASE phase, uint32_t frequencyHz);

/*Brief: SPI transmit
 * [in] - obj - pointer to SPI object
 * [in] - txBuffer - pointer to tx buffer
 * [in] - size - tx buffer size
 * [out] - none
 * */
void SpiTransmit(Spi_t* const obj, const uint8_t* const txBuffer, uint32_t size);

/*Brief: SPI receive
 * [in] - obj - pointer to SPI object
 * [in] - rxBuffer - pointer to rx buffer
 * [in] - size - rx buffer size
 * [out] - none
 * */
void SpiReceive(Spi_t* const obj, uint8_t* const rxBuffer, uint32_t size);

/*Brief: SPI transmit/receive
 * [in] - obj - pointer to SPI object
 * [in] - txBuffer - pointer to tx buffer
 * [in] - rxBuffer - pointer to rx buffer
 * [in] - size - rx buffer size
 * [out] - none
 * */
void SpiTransmitReceive(Spi_t* const obj, const uint8_t* const txBuffer, uint8_t* const rxBuffer, uint32_t size);

#endif /* SPI_H */
