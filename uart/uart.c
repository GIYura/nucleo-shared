#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "uart.h"
#include "gpio.h"

#define UART_1_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_USART1EN))
#define UART_6_CLOCK_ENABLE (RCC->APB2ENR |= (RCC_APB2ENR_USART6EN))

static Gpio_t m_GpioTx;
static Gpio_t m_GpioRx;

static bool m_initialized = false;

/*Brief: Send single character */
static void PutChar(const Uart_t* const obj, char ch)
{
    while (!(obj->usart->SR & USART_SR_TXE)){}
    {
        obj->usart->DR = ch;
    }
}

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

    /* enable USART */
    obj->usart->CR1 |= USART_CR1_UE;

    m_initialized = true;
}

void UartWrite(const Uart_t* const obj, const char* buff)
{
    assert(obj != NULL);
    assert(buff != NULL);

    if (!m_initialized)
    {
        return;
    }

    while (*buff)
    {
        PutChar(obj, *buff);
        buff++;
    }
}
