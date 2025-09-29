#include <stddef.h>

#include "assert.h"
#include "i2c.h"
#include "ignore.h"

typedef enum
{
    I2C_STANDARD_MODE = 0,
    I2C_FAST_MODE,
} I2C_MODES;

#define I2C_1_CLOCK_ENABLE (RCC->APB1ENR |= (RCC_APB1ENR_I2C1EN))
#define I2C_2_CLOCK_ENABLE (RCC->APB1ENR |= (RCC_APB1ENR_I2C2EN))
#define I2C_3_CLOCK_ENABLE (RCC->APB1ENR |= (RCC_APB1ENR_I2C3EN))

#define I2C_1_CLOCK_DISABLE (RCC->APB1ENR &= ~(RCC_APB1ENR_I2C1EN))
#define I2C_2_CLOCK_DISABLE (RCC->APB1ENR &= ~(RCC_APB1ENR_I2C2EN))
#define I2C_3_CLOCK_DISABLE (RCC->APB1ENR &= ~(RCC_APB1ENR_I2C3EN))

static I2C_Handle_t* m_I2CIrq[I2C_COUNT];
static uint32_t AHB_PRESCALERS[8] = { 2,4,8,16,64,128,256,512 };
static uint32_t APB1_PRESCALERS[4] = { 2,4,8,16 };

static void I2C_IrqEventHandler(I2C_Handle_t* const obj);
static void I2C_IrqErrorHandler(I2C_Handle_t* const obj);

static void I2C_WaitOnBusyFlag(const I2C_Handle_t* const obj);
static void I2C_WaitOnStartFlag(const I2C_Handle_t* const obj);
static void I2C_WaitOnAddrFlag(const I2C_Handle_t* const obj);
static void I2C_WaitOnTrasferFinished(const I2C_Handle_t* const obj);
static void I2C_ClearAddrFlag(const I2C_Handle_t* const obj);
static void I2C_DisableAck(const I2C_Handle_t* const obj);
static void I2C_EnableAck(const I2C_Handle_t* const obj);

static void I2C_Start(const I2C_Handle_t* const obj);
static void I2C_Stop(const I2C_Handle_t* const obj);

static void I2C_Enable(const I2C_Handle_t* const obj);
static void I2C_Disable(const I2C_Handle_t* const obj);

static void I2C_EnableEventInterrupt(const I2C_Handle_t* const obj);
static void I2C_EnableErrorInterrupt(const I2C_Handle_t* const obj);
static void I2C_EnableBufferInterrupt(const I2C_Handle_t* const obj);
static void I2C_DisableEventInterrupt(const I2C_Handle_t* const obj);
static void I2C_DisableErrorInterrupt(const I2C_Handle_t* const obj);
static void I2C_DisableBufferInterrupt(const I2C_Handle_t* const obj);
static IRQn_Type GetIrqEventType(const I2C_Handle_t* const obj);
static bool GetBitState(uint16_t reg, uint16_t bitMask)
{
    return (reg & bitMask) ? true : false;
}

static void I2C_WaitOnBusyFlag(const I2C_Handle_t* const obj)
{
    ASSERT(obj);
    while ((obj->instance->SR2 & I2C_SR2_BUSY));
}

static void I2C_WaitOnStartFlag(const I2C_Handle_t* const obj)
{
    ASSERT(obj);
    while (!(obj->instance->SR1 & I2C_SR1_SB));
}

static void I2C_WaitOnAddrFlag(const I2C_Handle_t* const obj)
{
    ASSERT(obj);
    while (!(obj->instance->SR1 & I2C_SR1_ADDR));
}

static void I2C_ClearAddrFlag(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    IGNORE(obj->instance->SR1);
    IGNORE(obj->instance->SR2);
}

static void I2C_Start(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR1 |= (I2C_CR1_START);
}

static void I2C_Stop(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR1 |= (I2C_CR1_STOP);
}

static void I2C_WaitOnTrasferFinished(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    while (!(obj->instance->SR1 & I2C_SR1_BTF));
}

static void I2C_WriteAddress(const I2C_Handle_t* const obj, uint8_t address)
{
    ASSERT(obj);

    obj->instance->DR = address;
}

static void I2C_EnableAck(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR1 |= (I2C_CR1_ACK);
}

static void I2C_DisableAck(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR1 &= ~(I2C_CR1_ACK);
}

static void I2C_Enable(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR1 |= (I2C_CR1_PE);
}

static void I2C_Disable(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR1 &= ~(I2C_CR1_PE);
}

static IRQn_Type GetIrqEventType(const I2C_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    IRQn_Type result;

    switch (obj->name)
    {
        case I2C_1:
            result = I2C1_EV_IRQn;
            break;

        case I2C_2:
            result = I2C2_EV_IRQn;
            break;

        case I2C_3:
            result = I2C3_EV_IRQn;
            break;

        default:
            ASSERT(false);
            break;
    }

    return result;
}


static uint32_t RCC_GetPLLOutputClock(void)
{
    /*TODO:*/
}

static uint8_t RCC_GetFrequency(void)
{
    uint32_t clockSource = (RCC->CFGR >> 2) & 0x03;
    uint32_t ahbPrescaler = (RCC->CFGR >> 4) & 0x0f;
    uint32_t apb1Prescaler = (RCC->CFGR >> 10) & 0x07;
    uint32_t ahbDevisor = 0;
    uint32_t apb1Devisor = 0;
    uint32_t systemClock = 0;

    uint32_t pclk1 = 0;

    switch (clockSource)
    {
        case 0:
            systemClock = 16000000;
            break;

        case 1:
            systemClock = 8000000;
            break;

        case 2:
            systemClock = RCC_GetPLLOutputClock();
            break;
        default:
            ASSERT(false);
            break;
    }

    if (ahbPrescaler < 8)
    {
        ahbDevisor = 1;
    }
    else
    {
        ahbDevisor = AHB_PRESCALERS[ahbPrescaler - 8];
    }

    if (apb1Prescaler < 4)
    {
        apb1Devisor = 1;
    }
    else
    {
        apb1Devisor = APB1_PRESCALERS[apb1Prescaler - 4];
    }

    pclk1 = (systemClock / ahbDevisor) / apb1Devisor;

    return (pclk1 / 1000000U);
}

static void I2C_Speed(I2C_Handle_t* const obj)
{
    ASSERT(obj);

    /*enter reset mode*/
    obj->instance->CR1 |= (I2C_CR1_SWRST);

    /*exit reset mode*/
    obj->instance->CR1 &= ~(I2C_CR1_SWRST);

    /*clock frequency*/
    obj->instance->CR2 |= (uint8_t)(RCC_GetFrequency() & 0x3F);//(I2C_CR2_FREQ_4); /*16 Mhz*/

    //obj->instance->CCR = 80;

    //obj->instance->TRISE = 17;
}

static void I2C_RiseTime(I2C_Handle_t* const obj)
{
    ASSERT(obj);

    uint8_t trise = (RCC_GetFrequency() + 1) & 0x3F;

    obj->instance->TRISE = trise;
}

static void I2C_EnableACK(I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR1 |= I2C_CR1_ACK;
}

static void I2C_DisableACK(I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR1 &= ~I2C_CR1_ACK;
}

static void I2C_Mode(I2C_Handle_t* const obj, I2C_MODES mode)
{
    ASSERT(obj);

    uint16_t ccrValue = 0;

    if (mode == I2C_STANDARD_MODE)
    {
        obj->instance->CCR &= ~(I2C_CCR_FS);
        //ccrValue = I2C_GetFrequency() / (2 * );
        //ccrValue = ccrValue & 0x0FFF;
    }
    else
    {
        obj->instance->CCR |= (I2C_CCR_FS);
    }

    obj->instance->CCR = 80;
}

void I2C_Init(I2C_Handle_t* const obj, I2C_NAMES name/*, I2C_Config_t* config*/)
{
    ASSERT(obj);
    //ASSERT(config != NULL);
    ASSERT(name < I2C_COUNT);

    obj->initialized = false;
    obj->name = name;

    switch (obj->name)
    {
        case I2C_1:

            GpioInit(&obj->sda, PB_9, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_OPEN_DRAIN, PIN_AF_4);
            GpioInit(&obj->scl, PB_8, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_OPEN_DRAIN, PIN_AF_4);

            obj->instance = I2C1;

            m_I2CIrq[I2C_1] = obj;

            I2C_1_CLOCK_ENABLE;

            break;

        case I2C_2:

            GpioInit(&obj->sda, PB_11, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_OPEN_DRAIN, PIN_AF_4);
            GpioInit(&obj->scl, PB_10, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_OPEN_DRAIN, PIN_AF_4);

            obj->instance = I2C2;

            I2C_2_CLOCK_ENABLE;

            break;

        case I2C_3:

            GpioInit(&obj->sda, PC_9, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_OPEN_DRAIN, PIN_AF_4);
            GpioInit(&obj->scl, PA_8, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_OPEN_DRAIN, PIN_AF_4);

            obj->instance = I2C3;

            I2C_3_CLOCK_ENABLE;

            break;

        default:
            ASSERT(false);
            break;
    }

    /* I2C speed */
    I2C_Speed(obj);

    I2C_EnableACK(obj);

    I2C_Mode(obj, I2C_STANDARD_MODE);

    I2C_RiseTime(obj);

    NVIC_EnableIRQ(GetIrqEventType(obj));

    BufferCreate(&obj->queue, obj->transactions, sizeof(obj->transactions), sizeof(I2C_Transaction_t), false);

    obj->currentTransaction = NULL;

    /* I2C enable */
    I2C_Enable(obj);

    obj->initialized = true;
}

void I2C_Deinit(I2C_Handle_t* const obj)
{
    ASSERT(obj);
    ASSERT(obj->initialized);

    switch (obj->name)
    {
        case I2C_1:
            I2C_1_CLOCK_DISABLE;
            break;

        case I2C_2:
            I2C_2_CLOCK_DISABLE;
            break;

        case I2C_3:
            I2C_3_CLOCK_DISABLE;
            break;
        default:
            ASSERT(false);
            break;
    }

    I2C_Disable(obj);

    obj->initialized = false;
}

void I2C1_EV_IRQHandler(void)
{
    I2C_IrqEventHandler(m_I2CIrq[I2C_1]);
}

void I2C2_ER_IRQHandler(void)
{
    I2C_IrqErrorHandler(m_I2CIrq[I2C_1]);
}

static void I2C_IrqErrorHandler(I2C_Handle_t* const obj)
{
/* TODO: */
}

#if 0
NOTE: refer to RM0383 18.3.3 I2C master mode
#endif
static void I2C_IrqEventHandler(I2C_Handle_t* const obj)
{
    ASSERT(obj);

    if (obj->currentTransaction == NULL)
    {
        /* get address (pointer) to the current transaction */
        obj->currentTransaction = (I2C_Transaction_t*)BufferFront(&obj->queue);

        if (obj->currentTransaction == NULL)
        {
            return;
        }
    }

    I2C_Transaction_t* t = obj->currentTransaction;

    bool eventInterruptEnable = GetBitState(obj->instance->CR2, I2C_CR2_ITEVTEN);
    bool bufferInterruptEnable = GetBitState(obj->instance->CR2, I2C_CR2_ITBUFEN);

    bool eventFlag = GetBitState(obj->instance->SR1, I2C_SR1_SB);

    /* handle SB flag */
    if (eventInterruptEnable && eventFlag)
    {
        if (t->TxRxState == I2C_BUSY_TX)
        {
            I2C_WriteAddress(obj, ((t->devAddress << 1) & ~(1 << 0)));
        }
        else
        {
            I2C_WriteAddress(obj, ((t->devAddress << 1) | (1 << 0)));
        }
    }

    eventFlag = GetBitState(obj->instance->SR1, I2C_SR1_ADDR);

    /* handle ADDR flag */
    if (eventInterruptEnable && eventFlag)
    {
        if (t->TxRxState == I2C_BUSY_TX)
        {
            /* ADDR flag is set */
            I2C_ClearAddrFlag(obj);
        }
        else if (t->TxRxState == I2C_BUSY_RX)
        {
            /* master receiver: setup ACK/POS according to remaining bytes */
            if (t->rxLen == 0)
            {
                I2C_ClearAddrFlag(obj);
                I2C_Stop(obj);
            }
            else if (t->rxLen == 1)
            {
                /* disable ACK, clear ADDR, STOP must be set after clearing ADDR */
                I2C_DisableAck(obj);

                /* clear ADDR */
                I2C_ClearAddrFlag(obj);

                /* generate STOP as RM suggests for 1 byte */
                I2C_Stop(obj);
            }
            else if (t->rxLen == 2)
            {
                I2C_DisableAck(obj);

                /* POS = 1, clear ACK, clear ADDR */
                obj->instance->CR1 |= I2C_CR1_POS;

                I2C_ClearAddrFlag(obj);
                /* now wait for BTF in subsequent events */
            }
            else
            {
                /* number of bytes > 2 */
                I2C_EnableAck(obj);
                I2C_ClearAddrFlag(obj);
                /* will read by RXNE until only 3 remain, then handle via BTF */
            }
        }
    }

    eventFlag = GetBitState(obj->instance->SR1, I2C_SR1_BTF);

    /* handle BTF flag */
    if (eventInterruptEnable && eventFlag)
    {
        if (t->TxRxState == I2C_BUSY_TX)
        {
            if (t->txLen == 0)
            {
                __disable_irq();
                I2C_Stop(obj);
                __enable_irq();

                /* TX transaction finished
                 * - set state IDLE
                 * - clear current transaction
                 * - extract current transaction from the transaction queue
                 * - disable interrupts
                 * - invoke callback on transaction done */
                t->TxRxState = I2C_IDLE;
                obj->currentTransaction = NULL;
                I2C_DisableEventInterrupt(obj);
                I2C_DisableErrorInterrupt(obj);
                I2C_DisableBufferInterrupt(obj);
                I2C_Transaction_t transaction;
                BufferGet(&obj->queue, &transaction, sizeof(I2C_Transaction_t));
                if (t->onTxDone != NULL)
                {
                    (*t->onTxDone)(t->context);
                }
            }
            /*TODO: else
            {

            }*/
        }
        else if (t->TxRxState == I2C_BUSY_RX)
        {
            if (t->rxLen == 3)
            {
                I2C_DisableAck(obj);

                *t->rxBuffer++ = (uint8_t)obj->instance->DR;
                t->rxLen--;
            }
            else if (t->rxLen == 2)
            {
                __disable_irq();
                I2C_Stop(obj);

                *t->rxBuffer++ = (uint8_t)obj->instance->DR;
                t->rxLen--;
                *t->rxBuffer++ = (uint8_t)obj->instance->DR;
                t->rxLen--;

                __enable_irq();

                t->TxRxState = I2C_IDLE;
                obj->currentTransaction = NULL;
                I2C_DisableEventInterrupt(obj);
                I2C_DisableErrorInterrupt(obj);
                I2C_DisableBufferInterrupt(obj);
                I2C_Transaction_t transaction;
                BufferGet(&obj->queue, &transaction, sizeof(I2C_Transaction_t));
                if (t->onRxDone != NULL)
                {
                    (*t->onRxDone)(t->context);
                }
            }
            else
            {
                /* For other cases BTF may be spurious or due to double buffering - ignore here */
            }
        }
    }

/* NOTE: use in slave mode only */
#if 0
    eventFlag = GetBitState(obj->instance->SR1, I2C_SR1_STOPF);

    if (eventInterruptEnable && eventFlag)
    {
        /* STOPF flag is set */
        I2C_ClearAddrFlag(obj);

        obj->instance->CR1 |= 0;
        /*notify appl. that master competes transaction */
    }
#endif

    eventFlag = GetBitState(obj->instance->SR1, I2C_SR1_TXE);

    /* handle TXE flag */
    if (eventInterruptEnable && bufferInterruptEnable && eventFlag)
    {
        if (t->TxRxState == I2C_BUSY_TX)
        {
            if (t->txLen > 0)
            {
                obj->instance->DR = *t->txBuffer++;
                t->txLen--;
            }
            /* else nothing to write; BTF handler will generate STOP when transfer truly finished */
        }
    }

    eventFlag = GetBitState(obj->instance->SR1, I2C_SR1_RXNE);

    /* handle RNXE flag */
    if (eventInterruptEnable && bufferInterruptEnable && eventFlag)
    {
        if (t->TxRxState == I2C_BUSY_RX)
        {
            /* if > 3 bytes, rely on RXNE */
            if (t->rxLen > 3)
            {
                *t->rxBuffer++ = (uint8_t)obj->instance->DR;
                t->rxLen--;
            }
            else if (t->rxLen == 1)
            {
                *t->rxBuffer++ = (uint8_t)obj->instance->DR;
                t->rxLen--;

                /* RX transaction finished
                 * - set state IDLE
                 * - clear current transaction
                 * - extract current transaction from the transaction queue
                 * - disable interrupts
                 * - invoke callback on transaction done */
                t->TxRxState = I2C_IDLE;
                obj->currentTransaction = NULL;
                I2C_DisableEventInterrupt(obj);
                I2C_DisableErrorInterrupt(obj);
                I2C_DisableBufferInterrupt(obj);
                I2C_Transaction_t transaction;
                BufferGet(&obj->queue, &transaction, sizeof(I2C_Transaction_t));
                if (t->onRxDone != NULL)
                {
                    (*t->onRxDone)(t->context);
                }
            }
        }
    }
}

void I2C_MasterTransmit(I2C_Handle_t* const obj, const uint8_t* txBuffer, uint8_t len, uint8_t slaveAddr)
{
    ASSERT(obj);
    ASSERT(txBuffer != NULL);
    ASSERT(obj->initialized);
    ASSERT(len > 0);

    /* 1. wait until bus is idle */
    I2C_WaitOnBusyFlag(obj);

    /* 2. start condition */
    I2C_Start(obj);

    /* 3. wait for SB flag. Read SR1 register */
    I2C_WaitOnStartFlag(obj);

    /* 4. address phase (send slave address) read/write bit = 0 */
    slaveAddr = (slaveAddr << 1) & ~(1 << 0);
    I2C_WriteAddress(obj, slaveAddr);

    /* 5. confirm address phase completed, clear ADDR flag */
    I2C_WaitOnAddrFlag(obj);
    I2C_ClearAddrFlag(obj);

    /* 6. send data */
    while (len > 0)
    {
        while (!(obj->instance->SR1 & I2C_SR1_TXE));
        obj->instance->DR = *txBuffer++;
        len--;

        if ((len > 0) && (obj->instance->SR1 & I2C_SR1_BTF))
        {
            obj->instance->DR = *txBuffer++;
            len--;
        }
    }

    /* 7. wait the last byte transmitted */
    I2C_WaitOnTrasferFinished(obj);

    /* 8. stop condition */
    I2C_Stop(obj);
}

void I2C_MasterReceive(I2C_Handle_t* const obj, uint8_t* rxBuffer, uint8_t len, uint8_t slaveAddr)
{
    ASSERT(obj);
    ASSERT(rxBuffer != NULL);
    ASSERT(obj->initialized);

    /* 1. wait until bus is idle */
    I2C_WaitOnBusyFlag(obj);

    /* 2. start condition */
    I2C_Start(obj);

    /* 3. wait for SB flag. Read SR1 register */
    I2C_WaitOnStartFlag(obj);

    /* 4. address phase (send slave address) read/write bit = 1 */
    slaveAddr = (slaveAddr << 1) | (1 << 0);
    I2C_WriteAddress(obj, slaveAddr);

    /* 5. confirm address phase completed */
    I2C_WaitOnAddrFlag(obj);

    /* case 0: nothing to receive */
    if (len == 0)
    {
        /* clear ADDR flag by reading SR1->SR2 */
        I2C_ClearAddrFlag(obj);

        /* stop condition */
        I2C_Stop(obj);

    }
    /* case 1: single byte to receive */
    else if (len == 1)
    {
        /* disable ACK */
        I2C_DisableAck(obj);

        __disable_irq();
        /* clear ADDR flag by reading SR1->SR2 */
        I2C_ClearAddrFlag(obj);
        /* set stop condition */
        I2C_Stop(obj);
        __enable_irq();

        /* wait RXNE */
        while (!(obj->instance->SR1 & I2C_SR1_RXNE));

        /* read data */
        *rxBuffer++ = obj->instance->DR;
        len--;
    }
    /* case 2: two bytes to receive */
    else if (len == 2)
    {
        /* disable ACK */
        I2C_DisableAck(obj);

        /* set POS */
        obj->instance->CR1 |= I2C_CR1_POS;

        /* clear ADDR flag by reading SR1->SR2 */
        __disable_irq();
        I2C_ClearAddrFlag(obj);
        __enable_irq();

        I2C_WaitOnTrasferFinished(obj);

        __disable_irq();
        I2C_Stop(obj);
        __enable_irq();

        *rxBuffer++ = obj->instance->DR;
        len--;
        *rxBuffer++ = obj->instance->DR;
        len--;

        obj->instance->CR1 &= ~I2C_CR1_POS;
    }
    /* case 3: receive > 2 bytes */
    else
    {
        /* enable ACK */
        I2C_EnableAck(obj);

        /* clear ADDR flag by reading SR1->SR2 */
        I2C_ClearAddrFlag(obj);

        while (len > 3)
        {
            while (!(obj->instance->SR1 & I2C_SR1_RXNE));
            *rxBuffer++ = obj->instance->DR;
            len--;
        }

        /* 3 bytes left to receive */
        I2C_WaitOnTrasferFinished(obj);

        __disable_irq();
        /* disable ACK */
        I2C_DisableAck(obj);

        *rxBuffer++ = obj->instance->DR;
        len--;
        __enable_irq();

        I2C_WaitOnTrasferFinished(obj);

        __disable_irq();
        I2C_Stop(obj);
        __enable_irq();

        *rxBuffer++ = obj->instance->DR;
        len--;
        *rxBuffer++ = obj->instance->DR;
        len--;
    }

    /* enable ACK */
    I2C_EnableAck(obj);
}

void I2C_Recovery(I2C_Handle_t* const obj)
{
    ASSERT(obj);
    ASSERT(obj->initialized);

    /*TODO:*/
}

I2C_RESULT I2C_MasterTransmit_IT(I2C_Handle_t* const obj, I2C_Transaction_t* transaction)
{
    ASSERT(obj != NULL);
    ASSERT(transaction != NULL);
    //ASSERT(obj->initialized);

    if (!obj->initialized)
    {
        return I2C_ERROR;
    }

    transaction->TxRxState = I2C_BUSY_TX;

    if (!BufferPut(&obj->queue, transaction, sizeof(I2C_Transaction_t)))
    {
        return I2C_QUEUE_FULL;
    }

    I2C_EnableEventInterrupt(obj);
    I2C_EnableErrorInterrupt(obj);
    I2C_EnableBufferInterrupt(obj);

    I2C_Start(obj);

    return I2C_OK;
}

I2C_RESULT I2C_MasterReceive_IT(I2C_Handle_t* const obj, I2C_Transaction_t* transaction)
{
    ASSERT(obj != NULL);
    ASSERT(transaction != NULL);

    if (!obj->initialized)
    {
        return I2C_ERROR;
    }

    transaction->TxRxState = I2C_BUSY_RX;

    if (!BufferPut(&obj->queue, transaction, sizeof(I2C_Transaction_t)))
    {
        return I2C_QUEUE_FULL;
    }

    I2C_EnableEventInterrupt(obj);
    I2C_EnableErrorInterrupt(obj);
    I2C_EnableBufferInterrupt(obj);

    I2C_Start(obj);

    return I2C_OK;
}

static void I2C_EnableEventInterrupt(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR2 |= (I2C_CR2_ITEVTEN);
}

static void I2C_EnableErrorInterrupt(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR2 |= (I2C_CR2_ITERREN);
}

static void I2C_EnableBufferInterrupt(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR2 |= (I2C_CR2_ITBUFEN);
}

static void I2C_DisableEventInterrupt(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR2 &= ~(I2C_CR2_ITEVTEN);
}

static void I2C_DisableErrorInterrupt(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR2 &= ~(I2C_CR2_ITERREN);
}

static void I2C_DisableBufferInterrupt(const I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR2 &= ~(I2C_CR2_ITBUFEN);
}
