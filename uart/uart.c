#include <stdbool.h>
#include <stddef.h>

#include "assert.h"
#include "uart.h"

#define UART_1_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_USART1EN))
#define UART_2_CLOCK_ENABLE (RCC->APB1ENR |= (RCC_APB1ENR_USART2EN))
#define UART_6_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_USART6EN))

#define DMA_1_CLOCK_ENABLE (RCC->AHB1ENR |= (RCC_AHB1ENR_DMA1EN))
#define DMA_2_CLOCK_ENABLE (RCC->AHB1ENR |= (RCC_AHB1ENR_DMA2EN))

static UART_Handle_t* m_UartIrq[UART_COUNT];

static void TxInterruptEnable(UART_Handle_t* const obj);
static void TxInterruptDisable(UART_Handle_t* const obj);

static void TcInterruptEnable(UART_Handle_t* const obj);
static void TcInterruptDisable(UART_Handle_t* const obj);

static IRQn_Type GetIrqType(const UART_Handle_t* const obj);

static void UartOnInterrupt(UART_Handle_t* const obj);

static void TransmitterEnable(UART_Handle_t* const obj);
static void ReceiverEnable(UART_Handle_t* const obj);

static void SetFormat(UART_Handle_t* const obj);

static void UartEnable(UART_Handle_t* const obj);

static void DMA_Config(UART_Handle_t* const obj, UART_NAMES uartName);

/*Brief: Converts baud rate in to register value */
static uint16_t ComputeBaudRate(uint32_t pclk, BAUD_RATE baud)
{
    uint32_t baudrate = 0;

    switch (baud)
    {
        case BAUD_1200:
            baudrate = 1200;
            break;

        case BAUD_2400:
            baudrate = 2400;
            break;

        case BAUD_9600:
            baudrate = 9600;
            break;

        case BAUD_38400:
            baudrate = 38400;
            break;

        case BAUD_57600:
            baudrate = 57600;
            break;

        case BAUD_115200:
            baudrate = 115200;
            break;

        case BAUD_230400:
            baudrate = 230400;
            break;

        case BAUD_460800:
            baudrate = 460800;
            break;

        case BAUD_921600:
            baudrate = 921600;
            break;

        default:
            break;
    }

    float usartDiv = (float)pclk / (16.0f * baudrate);

    uint32_t mantissa = (uint32_t)usartDiv;
    uint32_t fraction = (uint32_t)((usartDiv - mantissa) * 16.0f + 0.5f);

    if (fraction > 15)
    {
        mantissa += 1;
        fraction = 0;
    }

    return (mantissa << 4) | (fraction & 0x0F);
}

void UartInit(UART_Handle_t* const obj, UART_NAMES uartName, BAUD_RATE baud)
{
    ASSERT(obj != NULL);
    ASSERT(uartName < UART_COUNT);
    ASSERT(baud < BAUD_COUNT);

    obj->isTransmitting = false;
    obj->uartName = uartName;
    obj->isTransmitCompeted = false;
    obj->initialized = false;

    switch (uartName)
    {
        case UART_1:
            GpioInit(&obj->gpio.tx, PA_9, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_7);
            GpioInit(&obj->gpio.rx, PA_10, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_7);

            UART_1_CLOCK_ENABLE;

            obj->instance = USART1;

            obj->instance->BRR = ComputeBaudRate(SystemCoreClock, baud);

            break;

        case UART_2:
            GpioInit(&obj->gpio.tx, PD_5, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_7);
            GpioInit(&obj->gpio.rx, PD_6, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_7);

            UART_2_CLOCK_ENABLE;

            obj->instance = USART2;

            obj->instance->BRR = ComputeBaudRate(SystemCoreClock, baud);

            break;

        case UART_6:
            GpioInit(&obj->gpio.tx, PA_11, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_8);
            GpioInit(&obj->gpio.rx, PA_12, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_8);

            UART_6_CLOCK_ENABLE;

            obj->instance = USART6;

            obj->instance->BRR = ComputeBaudRate(SystemCoreClock, baud);

            break;

        default:
            ASSERT(false);
            break;
    }

    TransmitterEnable(obj);

    ReceiverEnable(obj);

    SetFormat(obj);

    UartEnable(obj);

    BufferCreate(&obj->txBuffer, &obj->txData, sizeof(obj->txData), sizeof(uint8_t), true);

    obj->initialized = true;

    NVIC_EnableIRQ(GetIrqType(obj));

    m_UartIrq[obj->uartName] = obj;
}

void UartWrite(UART_Handle_t* const obj, const uint8_t* const buffer, uint8_t size)
{
    ASSERT(obj != NULL);

    for (uint8_t i = 0; i < size; i++)
    {
        BufferPut(&obj->txBuffer, &buffer[i], sizeof(uint8_t));
    }

    if (!obj->isTransmitting)
    {
        obj->isTransmitting = true;
        obj->isTransmitCompeted = false;

        TxInterruptEnable(obj);
    }
}

bool UartIdle(UART_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    return obj->isTransmitCompeted;
}

void USART1_IRQHandler(void)
{
    UartOnInterrupt(m_UartIrq[UART_1]);
}

void USART2_IRQHandler(void)
{
    UartOnInterrupt(m_UartIrq[UART_2]);
}

void USART6_IRQHandler(void)
{
    UartOnInterrupt(m_UartIrq[UART_6]);
}

static void TxInterruptEnable(UART_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR1 |= (USART_CR1_TXEIE);
}

static void TxInterruptDisable(UART_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR1 &= ~(USART_CR1_TXEIE);
}

static void TcInterruptEnable(UART_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR1 |= (USART_CR1_TCIE);
}

static void TcInterruptDisable(UART_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR1 &= ~(USART_CR1_TCIE);
}

static IRQn_Type GetIrqType(const UART_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    IRQn_Type result;

    switch (obj->uartName)
    {
        case UART_1:
            result = USART1_IRQn;
            break;

        case UART_2:
            result = USART2_IRQn;
            break;

        case UART_6:
            result = USART6_IRQn;
            break;

        default:
            ASSERT(false);
            break;
    }

    return result;
}

static void UartOnInterrupt(UART_Handle_t* const obj)
{
    uint8_t item = 0;

    /* TX handle */
    if ((obj->instance->SR & (USART_SR_TXE)) && (obj->instance->CR1 & (USART_CR1_TXEIE)))
    {
        if (BufferGet(&obj->txBuffer, &item, sizeof(item)))
        {
            obj->instance->DR = item;
        }
        else
        {
            TxInterruptDisable(obj);
            TcInterruptEnable(obj);
        }
    }
    /*TODO: RX */

    /* TX complete handle */
    if ((obj->instance->SR & (USART_SR_TC)) && (obj->instance->CR1 & (USART_CR1_TCIE)))
    {
        obj->instance->SR &= ~(USART_SR_TC);

        TcInterruptDisable(obj);

        obj->isTransmitting = false;
        obj->isTransmitCompeted = true;
    }
}

static void TransmitterEnable(UART_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR1 |= USART_CR1_TE;
}

static void ReceiverEnable(UART_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR1 |= USART_CR1_RE;
}

static void SetFormat(UART_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    /* format: 1 Start bit, 8 Data bits, n Stop bit */
    obj->instance->CR1 &= ~(USART_CR1_M);

    /* Parity control disabled */
    obj->instance->CR1 &= ~(USART_CR1_PCE);

    /* 1 Stop bit */
    obj->instance->CR2 &= ~(USART_CR2_STOP);

    /* oversampling by 16 */
    obj->instance->CR1 &= ~(USART_CR1_OVER8);
}

static void UartEnable(UART_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR1 |= USART_CR1_UE;
}

static void DMA_Config(UART_Handle_t* const obj, UART_NAMES uartName)
{
    ASSERT(obj != NULL);

    switch (uartName)
    {
        case UART_1:
            DMA_2_CLOCK_ENABLE;

            DMA2_Stream7->CR &= ~DMA_SxCR_EN;

            while (DMA2_Stream7->CR & DMA_SxCR_EN);

            DMA2_Stream7->PAR = obj->instance->DR;
            DMA2_Stream7->CR = (4U << DMA_SxCR_CHSEL_Pos)  /* Channel 4 */
                               | DMA_SxCR_PL_1             /* High priority */
                               | DMA_SxCR_DIR_0            /* Memory -> Peripheral */
                               | DMA_SxCR_MINC;            /* Increment memory */

            DMA2_Stream7->FCR = 0;  /* Direct mode (no FIFO) */

            break;

        case UART_2:
            DMA_1_CLOCK_ENABLE;

            DMA2_Stream6->CR &= ~DMA_SxCR_EN;

            while (DMA2_Stream6->CR & DMA_SxCR_EN);

            DMA2_Stream6->PAR = obj->instance->DR;
            DMA2_Stream6->CR = (4U << DMA_SxCR_CHSEL_Pos)  /* Channel 4 */
                               | DMA_SxCR_PL_1             /* High priority */
                               | DMA_SxCR_DIR_0            /* Memory -> Peripheral */
                               | DMA_SxCR_MINC;            /* Increment memory */

            DMA2_Stream6->FCR = 0;  /* Direct mode (no FIFO) */

            break;

        case UART_6:
            DMA_2_CLOCK_ENABLE;

            DMA2_Stream7->CR &= ~DMA_SxCR_EN;

            while (DMA2_Stream7->CR & DMA_SxCR_EN);

            DMA2_Stream7->PAR = obj->instance->DR;
            DMA2_Stream7->CR = (5U << DMA_SxCR_CHSEL_Pos)  /* Channel 5 */
                               | DMA_SxCR_PL_1             /* High priority */
                               | DMA_SxCR_DIR_0            /* Memory -> Peripheral */
                               | DMA_SxCR_MINC;            /* Increment memory */

            DMA2_Stream7->FCR = 0;  /* Direct mode (no FIFO) */

            break;

        default:
            ASSERT(false);
            break;
    }
}
