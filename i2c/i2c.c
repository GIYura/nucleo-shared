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
static void I2C_IrqEventHandler(I2C_Handle_t* const obj);
static void I2C_IrqErrorHandler(I2C_Handle_t* const obj);

static void I2C_Enable(I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR1 |= (I2C_CR1_PE);
}

static void I2C_Disable(I2C_Handle_t* const obj)
{
    ASSERT(obj);

    obj->instance->CR1 &= ~(I2C_CR1_PE);
}

static uint32_t RCC_GetPLLOutputClock(void)
{
    /*TODO:*/
}

static uint32_t AHB_PRESCALERS[8] = { 2,4,8,16,64,128,256,512 };
static uint32_t APB1_PRESCALERS[4] = { 2,4,8,16 };

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

void I2C_Start(I2C_Handle_t* const obj)
{
    ASSERT(obj);
    ASSERT(obj->initialized);

    obj->instance->CR1 |= (I2C_CR1_START);

    while (!(obj->instance->SR1 & I2C_SR1_SB));
}

void I2C_Stop(I2C_Handle_t* const obj)
{
    ASSERT(obj);
    ASSERT(obj->initialized);

    obj->instance->CR1 |= (I2C_CR1_STOP);
}

void I2C_WriteAddress(I2C_Handle_t* const obj, uint8_t address)
{
    ASSERT(obj);
    ASSERT(obj->initialized);

    obj->instance->DR = address;

    while (!(obj->instance->SR1 & I2C_SR1_ADDR));
    IGNORE(obj->instance->SR2);
}

void I2C_WriteData(I2C_Handle_t* const obj, uint8_t data)
{
    ASSERT(obj);
    ASSERT(obj->initialized);

    while (!(obj->instance->SR1 & I2C_SR1_TXE));

    obj->instance->DR = data;

    while (!(obj->instance->SR1 & I2C_SR1_TXE));
}

void I2C_WaitTrasfetFinished(I2C_Handle_t* const obj)
{
    ASSERT(obj);
    ASSERT(obj->initialized);

    while (!(obj->instance->SR1 & I2C_SR1_BTF));
}

uint8_t I2C_ReadDataNACK(I2C_Handle_t* const obj)
{
    ASSERT(obj);
    ASSERT(obj->initialized);

    obj->instance->CR1 &= ~(I2C_CR1_ACK);

    IGNORE(obj->instance->SR2);

    obj->instance->CR1 |= (I2C_CR1_STOP);

    while (!(obj->instance->SR1 & I2C_SR1_RXNE));

    return obj->instance->DR;
}

void I2C_ReadAsync(I2C_Handle_t* const obj, uint8_t devAddr, uint8_t* data, uint8_t size)
{
    ASSERT(obj);
    ASSERT(obj->initialized);
    ASSERT(data != NULL);

    obj->devAddress = (devAddr << 1) | 1;
    obj->rxBuffer = data;
    obj->rxLen = size;
    obj->rxState = I2C_BUSY_RX;

    obj->instance->CR1 |= (I2C_CR1_START);
}

void I2C_WriteAsync(I2C_Handle_t* const obj, uint8_t devAddr, uint8_t* data, uint8_t size)
{
    ASSERT(obj);
    ASSERT(obj->initialized);
    ASSERT(data != NULL);

    obj->devAddress = (devAddr << 1);
    obj->txBuffer = data;
    obj->txLen = size;
    obj->txState = I2C_BUSY_TX;

    obj->instance->CR1 |= (I2C_CR1_START);
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

}

static void I2C_IrqEventHandler(I2C_Handle_t* const obj)
{
#if 0
    Interrupt handling for both master and slave mode of a device

    1. Handle For interrupt generated by SB event
      Note : SB flag is only applicable in Master mode

    2. Handle For interrupt generated by ADDR event
    Note : When master mode : Address is sent
           When Slave mode   : Address matched with own address

    3. Handle For interrupt generated by BTF(Byte Transfer Finished) event

    4. Handle For interrupt generated by STOPF event
     Note : Stop detection flag is applicable only slave mode . For master this flag will never be set

    5. Handle For interrupt generated by TXE event

    6. Handle For interrupt generated by RXNE event
#endif
    ASSERT(obj);

    if ((obj->instance->CR2 & I2C_CR2_ITEVTEN) && (obj->instance->SR1 & I2C_SR1_SB))
    {
        IGNORE(obj->instance->SR1);
        obj->instance->DR = obj->devAddress;
    }

    if ((obj->instance->CR2 & I2C_CR2_ITEVTEN) && (obj->instance->SR1 & I2C_SR1_ADDR))
    {
        IGNORE(obj->instance->SR2);
    }
}

void I2C_MasterTransmit(I2C_Handle_t* const obj, const uint8_t* txBuffer, uint8_t len, uint8_t slaveAddr)
{
    ASSERT(obj);
    ASSERT(txBuffer != NULL);
    ASSERT(obj->initialized);

    /* 1. start condition */
    obj->instance->CR1 |= (I2C_CR1_START);

    /* 2. wait for SB flag. Read SR1 register */
    while (!(obj->instance->SR1 & I2C_SR1_SB));

    /* 3. address phase (send slave address) read/write bit = 0 */
    slaveAddr = slaveAddr << 1;
    slaveAddr &= ~(1 << 0);
    obj->instance->DR = slaveAddr;

    /*NOTE: steps 2 and 3 clear SB flag */

    /* 4. confirm address phase completed, clear ADDR flag */
    while (!(obj->instance->SR1 & I2C_SR1_ADDR));
    IGNORE(obj->instance->SR2);

    /* 5.send data */
    while (len > 0)
    {
        obj->instance->DR = *txBuffer;
        txBuffer++;
        len--;

        while (!(obj->instance->SR1 & I2C_SR1_TXE));
    }

    /* 6. close transfer, wait until Byte Transfer Finished */
    while (!(obj->instance->SR1 & I2C_SR1_BTF));

    /* 7. stop condition */
    obj->instance->CR1 |= (I2C_CR1_STOP);
}

void I2C_MasterReceive(I2C_Handle_t* const obj, uint8_t* rxBuffer, uint8_t len, uint8_t slaveAddr)
{
    ASSERT(obj);
    ASSERT(rxBuffer != NULL);
    ASSERT(len != 0);

    /* 1. start condition */
    obj->instance->CR1 |= (I2C_CR1_START);

    /* 2. wait for SB flag. Read SR1 register */
    while (!(obj->instance->SR1 & I2C_SR1_SB));

    /* 3. address phase (send slave address) read/write bit = 1 */
    slaveAddr = slaveAddr << 1;
    slaveAddr |= (1 << 0);
    obj->instance->DR = slaveAddr;

    /* 4. confirm address phase completed */
    while (!(obj->instance->SR1 & I2C_SR1_ADDR));

    if (len == 1)
    {
        /* disable ACK */
        obj->instance->CR1 &= ~I2C_CR1_ACK;

        __disable_irq();
        /* clear ADDR flag by reading SR2 */
        IGNORE(obj->instance->SR2);

        /* stop condition */
        obj->instance->CR1 |= (I2C_CR1_STOP);
        __enable_irq();

        /* wait RXNE */
        while (!(obj->instance->SR1 & I2C_SR1_RXNE));

        /* read data */
        *rxBuffer = obj->instance->DR;
    }
    else if (len == 2)
    {
        obj->instance->CR1 |= I2C_CR1_POS;
        obj->instance->CR1 |= I2C_CR1_ACK;

        __disable_irq();
        IGNORE(obj->instance->SR2);
        obj->instance->CR1 &= ~I2C_CR1_ACK;

        while (!(obj->instance->SR1 & I2C_SR1_BTF));
        obj->instance->CR1 |= (I2C_CR1_STOP);
        __enable_irq();

        rxBuffer[0] = obj->instance->DR;
        rxBuffer[1] = obj->instance->DR;

        obj->instance->CR1 &= ~I2C_CR1_POS;
    }
    else
    {
        /*clear ADDR flag */
        obj->instance->CR1 |= I2C_CR1_ACK;
        IGNORE(obj->instance->SR2);

        while (len > 3)
        {
            while (!(obj->instance->SR1 & I2C_SR1_RXNE));
            *rxBuffer++ = obj->instance->DR;
            len--;
        }

        while (!(obj->instance->SR1 & I2C_SR1_BTF));

        obj->instance->CR1 &= ~I2C_CR1_ACK;

        __disable_irq();
        *rxBuffer++ = obj->instance->DR;
        obj->instance->CR1 |= (I2C_CR1_STOP);
        __enable_irq();

        while (!(obj->instance->SR1 & I2C_SR1_RXNE));
        *rxBuffer++ = obj->instance->DR;

        while (!(obj->instance->SR1 & I2C_SR1_RXNE));
        *rxBuffer++ = obj->instance->DR;
    }

    /* enable ACK */
    obj->instance->CR1 |= I2C_CR1_ACK;
}
