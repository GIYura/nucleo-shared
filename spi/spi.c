#include <assert.h>
#include <stddef.h>

#include "gpio.h"
#include "spi.h"

#define SPI_1_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_SPI1EN))
#define SPI_2_CLOCK_ENABLE (RCC->APB1ENR |= (RCC_APB1ENR_SPI2EN))
#define SPI_3_CLOCK_ENABLE (RCC->APB1ENR |= (RCC_APB1ENR_SPI3EN))
#define SPI_4_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_SPI4EN))
#define SPI_5_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_SPI5EN))

typedef struct
{
    Gpio_t mosi;
    Gpio_t miso;
    Gpio_t sck;
} SpiGpio_t;

static SpiGpio_t m_gpio;

void SpiMode(Spi_t* const obj, SPI_POLARITY polarity, SPI_PHASE phase)
{
    assert(obj != NULL);

    if (polarity == CPOL_0)
    {
       obj->instance->CR1 &= ~SPI_CR1_CPOL;
    }
    else
    {
        obj->instance->CR1 |= SPI_CR1_CPOL;
    }

    if (phase == CPHA_0)
    {
        obj->instance->CR1 &= ~SPI_CR1_CPHA;
    }
    else
    {
        obj->instance->CR1 |= SPI_CR1_CPHA;
    }
}

void SpiFormat(Spi_t* const obj)
{
    assert(obj != NULL);

    /* enable full duplex */
    obj->instance->CR1 &= ~SPI_CR1_RXONLY;

    /* MSB first */
    obj->instance->CR1 &= ~SPI_CR1_LSBFIRST;

    /* master mode */
    obj->instance->CR1 |= SPI_CR1_MSTR;

    /* 8 bit data */
    obj->instance->CR1 &= ~SPI_CR1_DFF;

    /* software slave management enabled */
    obj->instance->CR1 |= SPI_CR1_SSM;
    obj->instance->CR1 |= SPI_CR1_SSI;
}

void SpiSpeed(Spi_t* const obj, uint32_t frequencyHz)
{
    assert(obj != NULL);

    uint32_t divisor = 0;
    uint32_t spiClkFreq = SystemCoreClock;
    uint32_t baud = 0;

    while (spiClkFreq > frequencyHz)
    {
        divisor++;
        spiClkFreq = (spiClkFreq >> 1);
        if (divisor >= 7)
        {
            break;
        }
    }
    baud = (((divisor & 0x04) == 0) ? 0x00 : SPI_CR1_BR_2) |
           (((divisor & 0x02) == 0) ? 0x00 : SPI_CR1_BR_1) |
           (((divisor & 0x01) == 0) ? 0x00 : SPI_CR1_BR_0);

    obj->instance->CR1 |= baud << SPI_CR1_BR_Pos;
}

void SpiEnable(Spi_t* const obj)
{
    assert(obj != NULL);

    obj->instance->CR1 |= SPI_CR1_SPE;
}

void SpiInit(Spi_t* const obj, SPI_NAMES name, SPI_POLARITY polarity, SPI_PHASE phase, uint32_t frequencyHz)
{
    assert(obj != NULL);
    assert(frequencyHz != 0);

    obj->name = name;
    obj->initialized = false;

    switch (name)
    {
        case SPI_1:
            GpioInit(&m_gpio.mosi, PA_7, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_5);
            GpioInit(&m_gpio.miso, PA_6, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_5);
            GpioInit(&m_gpio.sck, PA_5, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_5);

            SPI_1_CLOCK_ENABLE;

            obj->instance = SPI1;

            break;

        case SPI_2:
            GpioInit(&m_gpio.mosi, PB_15, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_5);
            GpioInit(&m_gpio.miso, PB_14, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_5);
            GpioInit(&m_gpio.sck, PB_13, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_5);

            SPI_2_CLOCK_ENABLE;

            obj->instance = SPI2;

            break;

        case SPI_3:
            GpioInit(&m_gpio.mosi, PB_5, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_6);
            GpioInit(&m_gpio.miso, PB_4, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_6);
            GpioInit(&m_gpio.sck, PB_3, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_5);

            SPI_3_CLOCK_ENABLE;

            obj->instance = SPI3;

            break;

        case SPI_4:
            GpioInit(&m_gpio.mosi, PA_1, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_5);
            GpioInit(&m_gpio.miso, PA_11, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_6);
            GpioInit(&m_gpio.sck, PB_13, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_6);

            SPI_4_CLOCK_ENABLE;

            obj->instance = SPI4;

            break;

        case SPI_5:
            GpioInit(&m_gpio.mosi, PB_8, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_6);
            GpioInit(&m_gpio.miso, PA_12, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_5);
            GpioInit(&m_gpio.sck, PB_0, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_6);

            SPI_5_CLOCK_ENABLE;

            obj->instance = SPI5;

            break;

        default:
            assert(false);
            break;
    }

    SpiMode(obj, polarity, phase);
    SpiFormat(obj);
    SpiSpeed(obj, frequencyHz);
    SpiEnable(obj);

    obj->initialized = true;
}

void SpiTransmit(Spi_t* const obj, const uint8_t* const txBuffer, uint32_t size)
{
    assert(obj != NULL);
    assert(txBuffer != NULL);

    uint8_t dummy = 0;

    if (!obj->initialized)
    {
        return;
    }

    for (uint8_t i = 0; i < size; i++)
    {
        while (!(obj->instance->SR & SPI_SR_TXE));

        obj->instance->DR = txBuffer[i];

        while (obj->instance->SR & SPI_SR_BSY);

        dummy = obj->instance->DR;
        dummy = obj->instance->SR;
    }
}

void SpiReceive(Spi_t* const obj, uint8_t* const rxBuffer, uint32_t size)
{
    assert(obj != NULL);
    assert(rxBuffer != NULL);

    uint8_t dummy = 0xFF;

    if (!obj->initialized)
    {
        return;
    }

    for (uint8_t i = 0; i < size; i++)
    {
        obj->instance->DR = dummy;

        while (!(obj->instance->SR & SPI_SR_RXNE));

        rxBuffer[i] = obj->instance->DR;
    }
}

void SpiTransmitReceive(Spi_t* const obj, const uint8_t* const txBuffer, uint8_t* const rxBuffer, uint32_t size)
{
    assert(obj != NULL);
    assert(txBuffer != NULL);
    assert(rxBuffer != NULL);

    uint8_t dummy = 0xFF;

    if (!obj->initialized)
    {
        return;
    }

    for (uint8_t i = 0; i < size; i++)
    {
        while (!(obj->instance->SR & SPI_SR_TXE));

        obj->instance->DR = txBuffer[i];

        while (obj->instance->SR & SPI_SR_BSY);

        while (!(obj->instance->SR & SPI_SR_RXNE));

        rxBuffer[i] = obj->instance->DR;

        dummy = obj->instance->SR;
    }
}

