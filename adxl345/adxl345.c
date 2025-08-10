#include <stddef.h>

#include "adxl345.h"
#include "adxl345-regs.h"
#include "spi.h"

#define SPI_CLOCK_RATE  5000000     /* Hz */

static Spi_t m_spi;
static Gpio_t m_nss;

static void ADXL_ReadRegister(uint8_t address, uint8_t* const value);
static void ADXL_WriteRegister(uint8_t address, uint8_t value);

void ADXL_Init(void)
{
    SpiInit(&m_spi, SPI_1, CPOL_1, CPHA_1, SPI_CLOCK_RATE);

    /* chip select gpio */
    GpioInit(&m_nss, PA_0, PIN_MODE_OUTPUT, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, 1);
}

void ADXL_GetId(uint8_t* const id)
{
    uint8_t adxlId = 0;
    ADXL_ReadRegister(ADXL345_DEVID, &adxlId);

    *id = adxlId;
}

void ADXL_GetVector(void)
{
/*TODO: */
}

void ADXL_DumpRegisters(void)
{
    uint8_t regValue = 0;

    ADXL_ReadRegister(ADXL345_DEVID, &regValue);

    for (uint8_t regAddr = ADXL345_THRESH_TAP; regAddr <= ADXL345_FIFO_STATUS; regAddr++)
    {
        ADXL_ReadRegister(regAddr, &regValue);
    }
}

static void ADXL_ReadRegister(uint8_t address, uint8_t* const value)
{
    uint8_t dummy = 0xFF;
    uint8_t txBuffer[2] = { (0x80 | address), dummy };
    uint8_t rxBuffer[2] = { 0 };

    GpioWrite(&m_nss, 0);

    SpiTransfer(&m_spi, txBuffer, rxBuffer, sizeof(rxBuffer));

    *value = rxBuffer[1];

    GpioWrite(&m_nss, 1);
}

static void ADXL_WriteRegister(uint8_t address, uint8_t value)
{
    uint8_t txBuffer[2] = { (0x00 | address), value };

    GpioWrite(&m_nss, 0);

    SpiTransfer(&m_spi, txBuffer, NULL, sizeof(txBuffer));

    GpioWrite(&m_nss, 1);
}
