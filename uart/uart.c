#include <stdbool.h>

#include "uart.h"
#include "gpio.h"

#include "stm32f411xe.h"

static Gpio_t m_gpioTx;

static bool initialized = false;

void UartInit(void)
{
    GpioInit(&m_gpioTx, PA_9, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, PIN_AF_7);

    /*uart clock enable*/
    RCC->AHB2ENR |= (1 << 5);

    /*enable usart and tx*/
    USART1->CR1 |= (1 << 13) | (1 << 3);

    USART1->BRR = 0x1234;

    initialized = true;
}

void UartWrite(const uint8_t* const buff, uint8_t size)
{
    if (!initialized)
    {
        return;
    }

    for (uint8_t i = 0; i < size; i++)
    {
        USART1->DR = buff[i];
    }
}
