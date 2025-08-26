#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stdbool.h>

#include "stm32f411xe.h"

#include "gpio.h"

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
    Gpio_t miso;
    Gpio_t mosi;
    Gpio_t sck;
    Gpio_t nss;
} SpiGpio_t;

typedef void (*SpiEventHandler_t)(void* context);

typedef struct
{
    SPI_TypeDef* instance;
    SPI_NAMES name;
    SpiGpio_t gpio;
    uint8_t* txBuffer;          /* store appl. tx buffer address */
    uint8_t* rxBuffer;          /* store appl. rx buffer address */
    uint32_t txLen;
    uint32_t rxLen;
    uint8_t txState;
    uint8_t rxState;
    bool initialized;
    SpiEventHandler_t onRxDone;
    SpiEventHandler_t onTxDone;
    void* context;
} Spi_t;


/*Brief: SPI initialization
 * [in] - obj - pointer to SPI object
 * [in] - name - SPI name
 * [in] - polarity - SPI clock polarity
 * [in] - phase - SPI clock phase
 * [in] - deriredFrequencyHz - SPI frequency in Hz
 * [out] - actual SPI frequency in Hz
 * */
uint32_t SpiInit(Spi_t* const obj, SPI_NAMES name, SPI_POLARITY polarity, SPI_PHASE phase, uint32_t deriredFrequencyHz);

/*Brief: SPI de-initialization
 * [in] - obj - pointer to SPI object
 * [out] - none
 * */
void SpiDeinit(Spi_t* const obj);

/*Brief: SPI transmit/receive in blocking mode
 * [in] - obj - pointer to SPI object
 * [in] - txBuffer - buffer to transmit
 * [in] - rxBuffer - buffer to receive
 * [in] - size - buffer size
 * [out] - true - transfer successful; false - otherwise
 * */
bool SpiTransfer(Spi_t* const obj, const uint8_t* const txBuffer, uint8_t* const rxBuffer, uint8_t size);

/*Brief: SPI transmit/receive in non-blocking mode
 * [in] - obj - pointer to SPI object
 * [in] - txBuffer - buffer to transmit
 * [in] - rxBuffer - buffer to receive
 * [in] - size - buffer size
 * [in] - context - pointer to context (storage for response)
 * [out] - true - transfer successful; false - otherwise
 * */
uint8_t SpiTransfer_IT(Spi_t* const obj, uint8_t* txBuffer, uint8_t* rxBuffer, uint8_t size, void* context);

/*Brief: Register SPI on receive done callback
 * [in] - obj - pointer to SPI object
 * [in] - callback - callback on receive done
 * [out] - none
 * */
void SpiRegisterRxHandler(Spi_t* const obj, SpiEventHandler_t callback);

/*Brief: Register SPI on transmit done callback
 * [in] - obj - pointer to SPI object
 * [in] - callback - callback on transmit done
 * [out] - none
 * */
void SpiRegisterTxHandler(Spi_t* const obj, SpiEventHandler_t callback);

#endif /* SPI_H */
