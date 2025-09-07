#include <stdbool.h>
#include <stddef.h>

#include "assert.h"
#include "uart.h"

#define UART_1_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_USART1EN))
#define UART_2_CLOCK_ENABLE (RCC->APB1ENR |= (RCC_APB1ENR_USART2EN))
#define UART_6_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_USART6EN))

static UartHandle_t* m_UartIrq[UART_COUNT];

static void EnableTxInterrupt(UartHandle_t* const obj);
static void DisableTxInterrupt(UartHandle_t* const obj);

static void EnableTcInterrupt(UartHandle_t* const obj);
static void DisableTcInterrupt(UartHandle_t* const obj);

static IRQn_Type GetIrqType(const UartHandle_t* const obj);

static void UartOnInterrupt(UartHandle_t* const obj);

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

void UartInit(UartHandle_t* const obj, UART_NAMES uartName, BAUD_RATE baud)
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
            GpioInit(&obj->GpioTx, PA_9, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_7);
            GpioInit(&obj->GpioRx, PA_10, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_7);

            UART_1_CLOCK_ENABLE;

            obj->instance = USART1;

            obj->instance->BRR = ComputeBaudRate(SystemCoreClock, baud);

            break;

        case UART_2:
            GpioInit(&obj->GpioTx, PD_5, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_7);
            GpioInit(&obj->GpioRx, PD_6, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_7);

            UART_2_CLOCK_ENABLE;

            obj->instance = USART2;

            obj->instance->BRR = ComputeBaudRate(SystemCoreClock, baud);

            break;

        case UART_6:
            GpioInit(&obj->GpioTx, PA_11, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_8);
            GpioInit(&obj->GpioRx, PA_12, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_8);

            UART_6_CLOCK_ENABLE;

            obj->instance = USART6;

            obj->instance->BRR = ComputeBaudRate(SystemCoreClock, baud);

            break;

        default:
            ASSERT(false);
            break;
    }

    /* transmitter enable */
    obj->instance->CR1 |= USART_CR1_TE;

    /* receiver enable */
    obj->instance->CR1 |= USART_CR1_RE;

    /* format: 1 Start bit, 8 Data bits, n Stop bit */
    obj->instance->CR1 &= ~(USART_CR1_M);

    /* Parity control disabled */
    obj->instance->CR1 &= ~(USART_CR1_PCE);

    /* 1 Stop bit */
    obj->instance->CR2 &= ~(USART_CR2_STOP);

    /* oversampling by 16 */
    obj->instance->CR1 &= ~(USART_CR1_OVER8);

    /* enable USART */
    obj->instance->CR1 |= USART_CR1_UE;

    BufferCreate(&obj->txBuffer, &obj->txData, sizeof(obj->txData), sizeof(uint8_t), true);

    m_UartIrq[obj->uartName] = obj;

    obj->initialized = true;

    NVIC_EnableIRQ(GetIrqType(obj));
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

void UartWrite(UartHandle_t* const obj, uint8_t* buffer, uint8_t size)
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

        EnableTxInterrupt(obj);
    }
}

static void EnableTxInterrupt(UartHandle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR1 |= (USART_CR1_TXEIE);
}

static void DisableTxInterrupt(UartHandle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR1 &= ~(USART_CR1_TXEIE);
}

static void EnableTcInterrupt(UartHandle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR1 |= (USART_CR1_TCIE);
}

static void DisableTcInterrupt(UartHandle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->instance->CR1 &= ~(USART_CR1_TCIE);
}

static IRQn_Type GetIrqType(const UartHandle_t* const obj)
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

static void UartOnInterrupt(UartHandle_t* const obj)
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
            DisableTxInterrupt(obj);
            EnableTcInterrupt(obj);
        }
    }
    /*TODO: RX */

    /* TX complete handle */
    if ((obj->instance->SR & (USART_SR_TC)) && (obj->instance->CR1 & (USART_CR1_TCIE)))
    {
        obj->instance->SR &= ~(USART_SR_TC);

        DisableTcInterrupt(obj);

        obj->isTransmitting = false;
        obj->isTransmitCompeted = true;
    }
}

bool UartIdle(UartHandle_t* const obj)
{
    ASSERT(obj != NULL);

    return obj->isTransmitCompeted;
}

