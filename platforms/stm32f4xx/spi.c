#include <stddef.h>
#if 0
#include "custom-assert.h"
#include "delay.h"
#include "spi.h"
#include "ignore.h"

#define WAIT_FLAG_TIMEOUT_MAX   100 /* us */

#define SPI_1_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_SPI1EN))
#define SPI_2_CLOCK_ENABLE (RCC->APB1ENR |= (RCC_APB1ENR_SPI2EN))
#define SPI_3_CLOCK_ENABLE (RCC->APB1ENR |= (RCC_APB1ENR_SPI3EN))
#define SPI_4_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_SPI4EN))
#define SPI_5_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_SPI5EN))

static void SpiGpioInit(SPI_Handle_t* const obj, PIN_NAMES miso, PIN_NAMES mosi, PIN_NAMES sck);

static void SpiMode(const SPI_Handle_t* const obj, SPI_POLARITY polarity, SPI_PHASE phase);
static void SpiFormat(const SPI_Handle_t* const obj);

/*Brief: SPI speed
 * [in] - obj - pointer to SPI object
 * [in] - deriredFrequencyHz - desired SPI frequency in Hz
 * [out] - uint32_t value - actual SPI frequency in Hz
 * */
static uint32_t SpiSpeed(const SPI_Handle_t* const obj, uint32_t deriredFrequencyHz);
static void SpiClockEnable(const SPI_Handle_t* const obj);
static void SpiEnable(const SPI_Handle_t* const obj);
static void SpiDisable(const SPI_Handle_t* const obj);

static void SpiEnableTxInterrupt(const SPI_Handle_t* const obj);
static void SpiDisableTxInterrupt(const SPI_Handle_t* const obj);
static void SpiEnableRxInterrupt(const SPI_Handle_t* const obj);
static void SpiDisableRxInterrupt(const SPI_Handle_t* const obj);

static bool WaitFlagTimeout(volatile uint32_t* reg, uint32_t flag, bool state, uint32_t timeoutUs);
static void SpiClearOverrun(const SPI_Handle_t* const obj);

static void SpiIrqHandler(SPI_Handle_t* const obj);

static IRQn_Type GetIrqType(const SPI_Handle_t* const obj);

static SPI_Handle_t* m_SpiIrq[SPI_COUNT];

uint32_t SpiInit(SPI_Handle_t* const obj, SPI_NAMES name, SPI_POLARITY polarity, SPI_PHASE phase, uint32_t deriredFrequencyHz)
{
    ASSERT(obj != NULL);
    ASSERT(deriredFrequencyHz != 0);

    uint32_t actualFreq = 0;

    obj->name = name;
    obj->initialized = false;

    switch (name)
    {
        case SPI_1:
            SpiGpioInit(obj, PA_6, PA_7, PA_5);

            obj->instance = SPI1;

            m_SpiIrq[SPI_1] = obj;

            break;

        case SPI_2:
            SpiGpioInit(obj, PB_14, PB_15, PB_13);

            obj->instance = SPI2;

            m_SpiIrq[SPI_2] = obj;

            break;

        case SPI_3:
            SpiGpioInit(obj, PB_4, PB_5, PB_3);

            obj->instance = SPI3;

            m_SpiIrq[SPI_3] = obj;

            break;

        case SPI_4:
            SpiGpioInit(obj, PE_5, PE_6, PE_4);

            obj->instance = SPI4;

            m_SpiIrq[SPI_4] = obj;

            break;

        case SPI_5:
            SpiGpioInit(obj, PE_13, PE_14, PE_12);

            obj->instance = SPI5;

            m_SpiIrq[SPI_5] = obj;

            break;

        default:
            ASSERT(false);
            break;
    }

    SpiClockEnable(obj);

    SpiMode(obj, polarity, phase);

    SpiFormat(obj);

    actualFreq = SpiSpeed(obj, deriredFrequencyHz);

    SpiEnable(obj);

    SpiClearOverrun(obj);

    NVIC_EnableIRQ(GetIrqType(obj));

    BufferCreate(&obj->queue, obj->transactions, sizeof(obj->transactions), sizeof(SPI_Transaction_t), false);

    obj->currentTransaction = NULL;

    obj->initialized = true;

    return actualFreq;
}

void SpiDeinit(SPI_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->initialized = false;

    obj->currentTransaction = NULL;

    SpiDisable(obj);
}

bool SpiTransfer(SPI_Handle_t* const obj, const uint8_t* const txBuffer, uint8_t* const rxBuffer, uint8_t size)
{
    ASSERT(obj != NULL);

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

SPI_RESULT SpiTransfer_IT(SPI_Handle_t* const obj, SPI_Transaction_t* transaction)
{
    ASSERT(obj != NULL);
    ASSERT(transaction != NULL);

    if (!obj->initialized)
    {
        return SPI_ERROR;
    }

    if (!BufferPut(&obj->queue, transaction, sizeof(SPI_Transaction_t)))
    {
        return SPI_QUEUE_FULL;
    }

    SpiEnableRxInterrupt(obj);

    SpiEnableTxInterrupt(obj);

    return SPI_OK;
}

static void SpiGpioInit(SPI_Handle_t* const obj, PIN_NAMES miso, PIN_NAMES mosi, PIN_NAMES sck)
{
    ASSERT(obj != NULL);

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

static void SpiMode(const SPI_Handle_t* const obj, SPI_POLARITY polarity, SPI_PHASE phase)
{
    ASSERT(obj != NULL);

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

static void SpiFormat(const SPI_Handle_t* const obj)
{
    ASSERT(obj != NULL);

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

static uint32_t SpiSpeed(const SPI_Handle_t* const obj, uint32_t deriredFrequencyHz)
{
    ASSERT(obj != NULL);

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

static void SpiClockEnable(const SPI_Handle_t* const obj)
{
    ASSERT(obj != NULL);

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
            ASSERT(false);
            break;
    }
}

static void SpiEnable(const SPI_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR1 |= SPI_CR1_SPE;
}

static void SpiDisable(const SPI_Handle_t* const obj)
{
    ASSERT(obj != NULL);

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

static void SpiClearOverrun(const SPI_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    volatile uint32_t dummy = 0;

    dummy = obj->instance->DR;
    IGNORE(dummy);
    dummy = obj->instance->SR;
    IGNORE(dummy);
}

static void SpiIrqHandler(SPI_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    if (obj->currentTransaction == NULL)
    {
        SPI_Transaction_t next;

        if (!BufferGet(&obj->queue, &next, sizeof(SPI_Transaction_t)))
        {
            SpiDisableTxInterrupt(obj);
            SpiDisableRxInterrupt(obj);
            return;
        }

        obj->currentTransaction = &next;

        if (obj->currentTransaction->preTransaction != NULL)
        {
            (*obj->currentTransaction->preTransaction)(obj->currentTransaction->context);
        }
    }

    SPI_Transaction_t* t = obj->currentTransaction;

    if ((obj->instance->SR & SPI_SR_RXNE) && (obj->instance->CR2 & SPI_CR2_RXNEIE))
    {
        if (t->rxBuffer != NULL)
        {
            *(t->rxBuffer++) = (uint8_t)obj->instance->DR;
            t->rxLen--;
        }
        else
        {
            IGNORE(obj->instance->DR);
        }

        if (t->rxLen == 0 && t->txLen == 0)
        {
            SpiDisableTxInterrupt(obj);
            SpiDisableRxInterrupt(obj);

            if (t->postTransaction != NULL)
            {
                (*t->postTransaction)(NULL);
            }

            obj->currentTransaction = NULL;

            if (t->onTransactionDone != NULL)
            {
                (*t->onTransactionDone)(t->context);
            }
        }
    }

    if ((obj->instance->SR & SPI_SR_TXE) && (obj->instance->CR2 & SPI_CR2_TXEIE))
    {
        if (t->txLen > 0)
        {
            obj->instance->DR = *(t->txBuffer++);
            t->txLen--;
        }
    }

    if ((obj->instance->SR & SPI_SR_OVR) && (obj->instance->CR2 & SPI_CR2_ERRIE))
    {
        IGNORE(obj->instance->DR);
        IGNORE(obj->instance->SR);
    }
}

static IRQn_Type GetIrqType(const SPI_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    IRQn_Type result;

    switch (obj->name)
    {
        case SPI_1:
            result = SPI1_IRQn;
            break;

        case SPI_2:
            result = SPI2_IRQn;
            break;

        case SPI_3:
            result = SPI3_IRQn;
            break;

        case SPI_4:
            result = SPI4_IRQn;
            break;

        case SPI_5:
            result = SPI5_IRQn;
            break;

        default:
            ASSERT(false);
            break;
    }

    return result;
}

static void SpiEnableTxInterrupt(const SPI_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR2 |= SPI_CR2_TXEIE;
}

static void SpiDisableTxInterrupt(const SPI_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR2 &= ~SPI_CR2_TXEIE;
}

static void SpiEnableRxInterrupt(const SPI_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR2 |= SPI_CR2_RXNEIE;
}

static void SpiDisableRxInterrupt(const SPI_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR2 &= ~SPI_CR2_RXNEIE;
}

void SPI1_IRQHandler(void)
{
    SpiIrqHandler(m_SpiIrq[SPI_1]);
}

void SPI2_IRQHandler(void)
{
    SpiIrqHandler(m_SpiIrq[SPI_2]);
}

void SPI3_IRQHandler(void)
{
    SpiIrqHandler(m_SpiIrq[SPI_3]);
}

void SPI4_IRQHandler(void)
{
    SpiIrqHandler(m_SpiIrq[SPI_4]);
}

void SPI5_IRQHandler(void)
{
    SpiIrqHandler(m_SpiIrq[SPI_5]);
}
#endif
