#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "uart.h"
#include "gpio.h"

static Gpio_t m_gpio;

static bool m_initialized = false;

/*Brief: Send single character */
static void PutChar(const Usart_t* const obj, char ch)
{
    while (!(obj->usart->SR & USART_SR_TXE)){}
    {
        obj->usart->DR = ch;
    }
}

/*Brief: Converts baud rate in to register value */
static uint16_t BaudRateToRegisterValue(BAUD_RATE baud)
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

    float usartDiv = (float)SystemCoreClock / (16.0f * baudrate);

    uint32_t mantissa = (uint32_t)usartDiv;
    uint32_t fraction = (uint32_t)((usartDiv - mantissa) * 16.0f + 0.5f);

    if (fraction > 15)
    {
        mantissa += 1;
        fraction = 0;
    }

    return (mantissa << 4) | (fraction & 0x0F);
}

void UartInit(Usart_t* const obj, BAUD_RATE baud)
{
    assert(baud < BAUD_COUNT);
    assert(obj != NULL);

    /* init PA9 (USART1 TX) */
    GpioInit(&m_gpio, PA_9, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_7);

    /* link USART1 instance */
    obj->usart = USART1;

    /* USART clock enable */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    obj->usart->BRR = BaudRateToRegisterValue(baud);

    /* enable transmitter */
    obj->usart->CR1 |= USART_CR1_TE;

    /* enable USART */
    obj->usart->CR1 |= USART_CR1_UE;

    m_initialized = true;
}

void UartWrite(const Usart_t* const obj, const char* buff)
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
