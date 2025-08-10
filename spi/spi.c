#include <assert.h>
#include <stddef.h>

#include "delay.h"
#include "spi.h"
#include "ignore.h"

#define WAIT_FLAG_TIMEOUT_MAX   100 /* us */

#define SPI_1_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_SPI1EN))
#define SPI_2_CLOCK_ENABLE (RCC->APB1ENR |= (RCC_APB1ENR_SPI2EN))
#define SPI_3_CLOCK_ENABLE (RCC->APB1ENR |= (RCC_APB1ENR_SPI3EN))
#define SPI_4_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_SPI4EN))
#define SPI_5_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_SPI5EN))

static void SpiGpioInit(Spi_t* const obj, PIN_NAMES miso, PIN_NAMES mosi, PIN_NAMES sck);

static void SpiMode(const Spi_t* const obj, SPI_POLARITY polarity, SPI_PHASE phase);
static void SpiFormat(const Spi_t* const obj);

/*Brief: SPI speed
 * [in] - obj - pointer to SPI object
 * [in] - deriredFrequencyHz - desired SPI frequency in Hz
 * [out] - uint32_t value - actual SPI frequency in Hz
 * */
static uint32_t SpiSpeed(const Spi_t* const obj, uint32_t deriredFrequencyHz);
static void SpiClockEnable(const Spi_t* const obj);
static void SpiEnable(const Spi_t* const obj);
static void SpiDisable(const Spi_t* const obj);

static bool WaitFlagTimeout(volatile uint32_t* reg, uint32_t flag, bool state, uint32_t timeoutUs);
static void SpiClearOverrun(const Spi_t* const obj);

void SpiInit(Spi_t* const obj, SPI_NAMES name, SPI_POLARITY polarity, SPI_PHASE phase, uint32_t deriredFrequencyHz)
{
    assert(obj != NULL);
    assert(deriredFrequencyHz != 0);

    uint32_t actualFreq = 0;

    obj->name = name;
    obj->initialized = false;

    switch (name)
    {
        case SPI_1:
            SpiGpioInit(obj, PA_6, PA_7, PA_5);

            obj->instance = SPI1;

            break;

        case SPI_2:
            SpiGpioInit(obj, PB_14, PB_15, PB_13);

            obj->instance = SPI2;

            break;

        case SPI_3:
            SpiGpioInit(obj, PB_4, PB_5, PB_3);

            obj->instance = SPI3;

            break;

        case SPI_4:
            SpiGpioInit(obj, PE_5, PE_6, PE_4);

            obj->instance = SPI4;

            break;

        case SPI_5:
            SpiGpioInit(obj, PE_13, PE_14, PE_12);

            obj->instance = SPI5;

            break;

        default:
            assert(false);
            break;
    }

    SpiClockEnable(obj);

    SpiMode(obj, polarity, phase);

    SpiFormat(obj);

    actualFreq = SpiSpeed(obj, deriredFrequencyHz);

    SpiEnable(obj);

    SpiClearOverrun(obj);

    obj->initialized = true;
}

void SpiDeinit(Spi_t* const obj)
{
    obj->initialized = false;

    SpiDisable(obj);
}

bool SpiTransfer(Spi_t* const obj, const uint8_t* const txBuffer, uint8_t* const rxBuffer, uint8_t size)
{
    assert(obj != NULL);

    if (!obj->initialized)
    {
        return false;
    }

    uint32_t dummy = 0;

    for (uint8_t i = 0; i < size; i++)
    {
        if (!WaitFlagTimeout(&obj->instance->SR, SPI_SR_TXE, 1, WAIT_FLAG_TIMEOUT_MAX))
        {
            goto timeout;
        }

        obj->instance->DR = (txBuffer != NULL) ? txBuffer[i] : 0xFF;

        if (!WaitFlagTimeout(&obj->instance->SR, SPI_SR_RXNE, 1, WAIT_FLAG_TIMEOUT_MAX))
        {
            goto timeout;
        }

        if (rxBuffer != NULL)
        {
            rxBuffer[i] = obj->instance->DR;
        }
        else
        {
            dummy = obj->instance->DR;
            IGNORE(dummy);
        }
    }

    if (!WaitFlagTimeout(&obj->instance->SR, SPI_SR_BSY, 0, WAIT_FLAG_TIMEOUT_MAX))
    {
        goto timeout;
    }

    return true;

timeout:
    SpiClearOverrun(obj);

    return false;
}

static void SpiGpioInit(Spi_t* const obj, PIN_NAMES miso, PIN_NAMES mosi, PIN_NAMES sck)
{
    assert(obj != NULL);

    if (obj->name == SPI_3 || obj->name == SPI_5)
    {
        GpioInit(&obj->gpio.miso, miso, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_6);
        GpioInit(&obj->gpio.mosi, mosi, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_6);
        GpioInit(&obj->gpio.sck, sck, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_6);
    }
    else
    {
        GpioInit(&obj->gpio.miso, miso, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_5);
        GpioInit(&obj->gpio.mosi, mosi, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_5);
        GpioInit(&obj->gpio.sck, sck, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_5);
    }
}

static void SpiMode(const Spi_t* const obj, SPI_POLARITY polarity, SPI_PHASE phase)
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

static void SpiFormat(const Spi_t* const obj)
{
    assert(obj != NULL);

    /* master mode */
    obj->instance->CR1 |= (SPI_CR1_MSTR);

    /* MSB transmitted first */
    obj->instance->CR1 &= ~(SPI_CR1_LSBFIRST);

    /*Software slave management enabled  */
    obj->instance->CR1 |= (SPI_CR1_SSM);
    obj->instance->CR1 |= (SPI_CR1_SSI);

    /* Full duplex (Transmit and receive) */
    obj->instance->CR1 &= ~(SPI_CR1_RXONLY);

    /* 8-bit data frame format is selected for transmission/reception */
    obj->instance->CR1 &= ~(SPI_CR1_DFF);

    /* CRC calculation disabled */
    obj->instance->CR1 &= ~(SPI_CR1_CRCEN);

    /* Data phase (no CRC phase) */
    obj->instance->CR1 &= ~(SPI_CR1_CRCNEXT);

    /* 2-line unidirectional data mode selected */
    obj->instance->CR1 &= ~(SPI_CR1_BIDIMODE);
}

static uint32_t SpiSpeed(const Spi_t* const obj, uint32_t deriredFrequencyHz)
{
    uint32_t pclk = 0;
    uint32_t prescaler = 0;
    uint32_t brBits = 0;

    if (obj->name == SPI_1 || obj->name == SPI_4 || obj->name == SPI_5)
    {
        /* APB2 */
        uint32_t hclk = SystemCoreClock;
        uint32_t apb2Prescaler = ((RCC->CFGR >> RCC_CFGR_PPRE2_Pos) & 0x7);
        if (apb2Prescaler < 4)
        {
            apb2Prescaler = 1;
        }
        else
        {
            apb2Prescaler = 1 << (apb2Prescaler - 3);
        }
        pclk = hclk / apb2Prescaler;
    }
    else
    {
        /* APB1 */
        uint32_t hclk = SystemCoreClock;
        uint32_t apb1Prescaler = ((RCC->CFGR >> RCC_CFGR_PPRE1_Pos) & 0x7);
        if (apb1Prescaler < 4)
        {
            apb1Prescaler = 1;
        }
        else
        {
            apb1Prescaler = 1 << (apb1Prescaler - 3);
        }
        pclk = hclk / apb1Prescaler;
    }

    prescaler = pclk / deriredFrequencyHz;

    while ((prescaler > 2) && (brBits < 7))
    {
        prescaler >>= 1;
        brBits++;
    }

    obj->instance->CR1 &= ~SPI_CR1_BR_Msk;
    obj->instance->CR1 |= (brBits << SPI_CR1_BR_Pos);

    return pclk / (1U << (brBits + 1));
}

static void SpiClockEnable(const Spi_t* const obj)
{
    assert(obj != NULL);

    switch (obj->name)
    {
        case SPI_1:
            SPI_1_CLOCK_ENABLE;
            break;
        case SPI_2:
            SPI_2_CLOCK_ENABLE;
            break;
        case SPI_3:
            SPI_3_CLOCK_ENABLE;
            break;
        case SPI_4:
            SPI_4_CLOCK_ENABLE;
            break;
        case SPI_5:
            SPI_5_CLOCK_ENABLE;
            break;
        default:
            assert(0);
            break;
    }
}

static void SpiEnable(const Spi_t* const obj)
{
    assert(obj != NULL);

    obj->instance->CR1 |= SPI_CR1_SPE;
}

static void SpiDisable(const Spi_t* const obj)
{
    assert(obj != NULL);

    obj->instance->CR1 &= ~SPI_CR1_SPE;
}

static bool WaitFlagTimeout(volatile uint32_t* reg, uint32_t flag, bool state, uint32_t timeoutUs)
{
    while (((*reg & flag) ? 1 : 0) != (state ? 1 : 0))
    {
        if (timeoutUs == 0)
        {
            return false;
        }
        DelayUs(1);
        timeoutUs--;
    }
    return true;
}

static void SpiClearOverrun(const Spi_t* const obj)
{
    assert(obj != NULL);

    volatile uint32_t dummy = 0;

    dummy = obj->instance->DR;
    IGNORE(dummy);
    dummy = obj->instance->SR;
    IGNORE(dummy);
}
