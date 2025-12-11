#include <stddef.h>
#if 0
#include "custom-assert.h"
#include "uart-service.h"
#include "uart.h"

typedef struct
{
    UART_Handle_t uart;
    UartRxCallback_t callback;
} UartServiceHandle_t;

static UartServiceHandle_t m_uartService;

static void OnUartRxDone(void* context)
{
    UART_Handle_t* handle = (UART_Handle_t*)context;

    uint8_t buffer[64];
    uint8_t count = 0;
    uint8_t item = 0;
    uint8_t bufferSize = sizeof(buffer);

    while (BufferGet(&handle->rxBuffer, &item, sizeof(item)))
    {
        if (count < bufferSize)
        {
            buffer[count++] = item;
        }
        else
        {
            ASSERT(false);
        }
    }

    if (count > 0)
    {
        if (m_uartService.callback != NULL)
        {
            (*m_uartService.callback)(buffer, count);
        }
    }
}

void UartServiceInit(void)
{
    UartInit(&m_uartService.uart, UART_1, BAUD_921600);

    UartRegisterReceiveHandler(&m_uartService.uart, &OnUartRxDone);
}

void UartServiceSend(const uint8_t* const data, uint8_t len)
{
    ASSERT(data);

    UartWrite_IT(&m_uartService.uart, (uint8_t*)data, len);
}

void UartServiceRegisterRxCallback(UartRxCallback_t callback)
{
    m_uartService.callback = callback;
}

bool UartServiceIdle(void)
{
    return UartIdle(&m_uartService.uart);
}

#endif
