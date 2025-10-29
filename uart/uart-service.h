#ifndef UART_SERVICE_H
#define UART_SERVICE_H

#include <stdint.h>
#include <stdbool.h>

typedef void (*UartRxCallback_t)(const uint8_t* data, uint8_t len);

/*Brief: UART service initialization
 * [in] - none
 * [out] - none
 * */
void UartServiceInit(void);

/*Brief: UART service send data
 * [in] - data - pointer to data to send
 * [in] - len - data length
 * [out] - none
 * */
void UartServiceSend(const uint8_t* const data, uint8_t len);

/*Brief: Register receive callback
 * [in] - callback - callback
 * [out] - none
 * */
void UartServiceRegisterRxCallback(UartRxCallback_t callback);

/*Brief: Check UART service state
 * [in] - none
 * [out] - true - busy; false - otherwise
 * */
bool UartServiceIdle(void);

#endif /* UART_SERVICE_H */

