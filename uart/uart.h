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

typedef struct
{
    USART_TypeDef* usart;
} Usart_t;

/*Brief: USART initialization
 * [in] - obj - pointer to USART object
 * [in] - baud - baud rate
 * [out] - none
 * */
void UartInit(Usart_t* const obj, BAUD_RATE baud);

/*Brief: USART send buffer
 * [in] - obj - pointer to USART object
 * [in] - buff - pointer to string
 * [out] - none
 * */
void UartWrite(const Usart_t* const obj, const char* buff);

#endif /* UART_H */
