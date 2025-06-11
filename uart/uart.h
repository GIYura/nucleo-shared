#ifndef UART_H
#define UART_H

#include <stdint.h>

void UartInit(void);

void UartWrite(const uint8_t* const buff, uint8_t size);

#endif /* UART_H */
