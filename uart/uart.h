#ifndef UART_H
#define UART_H

#include <stdint.h>

#include "stm32f411xe.h"

/* USART baud rate supported */
typedef enum
{
    BAUD_1200 = 0,
    BAUD_2400,
    BAUD_9600,
    BAUD_19200,
    BAUD_38400,
    BAUD_57600,
    BAUD_115200,
    BAUD_230400,
    BAUD_460800,
    BAUD_921600,
    BAUD_COUNT
} BAUD_RATE;

typedef enum
{
    UART_1 = 0,
    UART_6,
    UART_COUNT
} UART_NAMES;

typedef struct
{
    USART_TypeDef* usart;
    UART_NAMES uartName;
} Uart_t;

/*Brief: UART initialization
 * [in] - obj - pointer to UART object
 * [in] - baud - baud rate
 * [out] - none
 * */
void UartInit(Uart_t* const obj, UART_NAMES uartName, BAUD_RATE baud);

/*Brief: UART send buffer
 * [in] - obj - pointer to UART object
 * [in] - buff - pointer to string
 * [out] - none
 * */
void UartWrite(const Uart_t* const obj, const char* buff);

#endif /* UART_H */
