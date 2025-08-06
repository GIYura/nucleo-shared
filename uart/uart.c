#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "uart.h"
#include "gpio.h"

#define UART_1_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_USART1EN))
#define UART_6_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_USART6EN))

static Gpio_t m_GpioTx;
static Gpio_t m_GpioRx;

static Uart_t* m_UartIrq[UART_COUNT];

static bool m_initialized = false;

static void EnableTxInterrupt(Uart_t* const obj);
static void DisableTxInterrupt(Uart_t* const obj);

static void EnableTcInterrupt(Uart_t* const obj);
static void DisableTcInterrupt(Uart_t* const obj);

static void UartStart(Uart_t* const obj);
static IRQn_Type GetIrqType(const Uart_t* const obj);
static void UartOnInterrupt(Uart_t* const obj);

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

void UartInit(Uart_t* const obj, UART_NAMES uartName, BAUD_RATE baud)
{
    assert(obj != NULL);
    assert(uartName < UART_COUNT);
    assert(baud < BAUD_COUNT);

    obj->isTransmitting = false;
    obj->uartName = uartName;
    obj->isTransmitCompeted = false;

    switch (uartName)
    {
        case UART_1:
            GpioInit(&m_GpioTx, PA_9, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_7);
            GpioInit(&m_GpioRx, PA_10, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_7);

            UART_1_CLOCK_ENABLE;

            obj->usart = USART1;

            obj->usart->BRR = ComputeBaudRate(SystemCoreClock, baud);

            break;

        case UART_6:
            GpioInit(&m_GpioTx, PA_11, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_8);
            GpioInit(&m_GpioRx, PA_12, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_8);

            UART_6_CLOCK_ENABLE;

            obj->usart = USART6;

            obj->usart->BRR = ComputeBaudRate(SystemCoreClock, baud);

            break;

        default:
            assert(false);
            break;
    }

    /* transmitter enable */
    obj->usart->CR1 |= USART_CR1_TE;

    /* receiver enable */
    obj->usart->CR1 |= USART_CR1_RE;

    /* format: 1 Start bit, 8 Data bits, n Stop bit */
    obj->usart->CR1 &= ~(USART_CR1_M);

    /* Parity control disabled */
    obj->usart->CR1 &= ~(USART_CR1_PCE);

    /* 1 Stop bit */
    obj->usart->CR2 &= ~(USART_CR2_STOP);

    /* oversampling by 16 */
    obj->usart->CR1 &= ~(USART_CR1_OVER8);

    /* enable USART */
    obj->usart->CR1 |= USART_CR1_UE;

    BufferCreate(&obj->txBuffer, &obj->txData, sizeof(obj->txData), sizeof(uint8_t), true);

    m_UartIrq[obj->uartName] = obj;

    m_initialized = true;

    NVIC_EnableIRQ(GetIrqType(obj));
}

void USART1_IRQHandler(void)
{
    UartOnInterrupt(m_UartIrq[UART_1]);
}

void USART6_IRQHandler(void)
{
    UartOnInterrupt(m_UartIrq[UART_6]);
}

void UartWrite(Uart_t* const obj, uint8_t* buffer, uint8_t size)
{
    for (uint8_t i = 0; i < size; i++)
    {
        BufferPut(&obj->txBuffer, &buffer[i], sizeof(uint8_t));
    }

    if (!obj->isTransmitting)
    {
        obj->isTransmitting = true;
        obj->isTransmitCompeted = false;

        UartStart(obj);
    }
}

static void EnableTxInterrupt(Uart_t* const obj)
{
    assert(obj != NULL);

    obj->usart->CR1 |= (USART_CR1_TXEIE);
}

static void DisableTxInterrupt(Uart_t* const obj)
{
    assert(obj != NULL);

    obj->usart->CR1 &= ~(USART_CR1_TXEIE);
}

static void EnableTcInterrupt(Uart_t* const obj)
{
    assert(obj != NULL);

    obj->usart->CR1 |= (USART_CR1_TCIE);
}

static void DisableTcInterrupt(Uart_t* const obj)
{
    assert(obj != NULL);

    obj->usart->CR1 &= ~(USART_CR1_TCIE);
}

static void UartStart(Uart_t* const obj)
{
    assert(obj != NULL);

    uint8_t item = 0;

    if (BufferGet(&obj->txBuffer, &item, sizeof(item)))
    {
        obj->usart->DR = item;

        EnableTxInterrupt(obj);
    }
    else
    {
        obj->isTransmitting = false;
        obj->isTransmitCompeted = true;
    }
}

static IRQn_Type GetIrqType(const Uart_t* const obj)
{
    assert(obj != NULL);

    IRQn_Type result;

    switch (obj->uartName)
    {
        case UART_1:
            result = USART1_IRQn;
            break;

        case UART_6:
            result = USART6_IRQn;
            break;

        default:
            assert(0);
            break;
    }

    return result;
}

static void UartOnInterrupt(Uart_t* const obj)
{
    uint8_t item = 0;

    /* TX handle */
    if ((obj->usart->SR & (USART_SR_TXE)) && (obj->usart->CR1 & (USART_CR1_TXEIE)))
    {
        if (BufferGet(&obj->txBuffer, &item, sizeof(item)))
        {
            obj->usart->DR = item;
        }
        else
        {
            DisableTxInterrupt(obj);
            EnableTcInterrupt(obj);
        }
    }
    /*TODO: RX */

    /* TX complete handle */
    if ((obj->usart->SR & (USART_SR_TC)) && (obj->usart->CR1 & (USART_CR1_TCIE)))
    {
        obj->usart->SR &= ~(USART_SR_TC);

        DisableTcInterrupt(obj);

        obj->isTransmitting = false;
        obj->isTransmitCompeted = true;
    }
}

bool UartIdle(Uart_t* const obj)
{
    return obj->isTransmitCompeted;
}

