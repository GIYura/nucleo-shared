#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

#include "stm32f411xe.h"

#include "buffer.h"

#define TX_BUFFER_SIZE      1023

/* USART baud rate supported */
#if 0
There might be an issue with writting data over UART on lower baudrate due to
circular buffer might be flooded with data.
Recommended baudrate: 921600.
#endif
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
    Buffer_t txBuffer;
    uint8_t txData[TX_BUFFER_SIZE + 1];
    volatile bool isTransmitting;
    volatile bool isTransmitCompeted;
} Uart_t;

/*Brief: UART initialization
 * [in] - obj - pointer to UART object
 * [in] - baud - baud rate
 * [out] - none
 * */
void UartInit(Uart_t* const obj, UART_NAMES uartName, BAUD_RATE baud);

/*Brief: Send message over UART
 * [in] - obj - pointer to UART object
 * [in] - buff - pointer to buffer
 * [in] - size - buffer size
 * [out] - none
 * */
void UartWrite(Uart_t* const obj, uint8_t* buffer, uint8_t size);

/*Brief: Check if UART is in Idle state
 * [in] - obj - pointer to UART object
 * [out] - true - idle (free); fale - otherwise (busy)
 * */
bool UartIdle(Uart_t* const obj);

#endif /* UART_H */
