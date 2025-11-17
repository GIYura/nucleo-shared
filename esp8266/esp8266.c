#include <string.h>

#include "buffer.h"
#include "esp8266.h"
#include "uart.h"

#define ESP_RESPONSE_MAX    128

static UART_Handle_t m_uart;
static uint8_t m_rxBuffer[ESP_RESPONSE_MAX];
static ESP_ResponseHandler_t m_onEspResponse = NULL;

static ESP_RESPONSE ESP_ParseResponse(const char* resp);

static void OnUartReceiveCompleted(void* context)
{
    UART_Handle_t* handle = (UART_Handle_t*)context;

    uint16_t count = BufferCount(&handle->rxBuffer);

    if (count < ESP_RESPONSE_MAX)
    {
        for (uint16_t i = 0; i < count; i++)
        {
            BufferGet(&handle->rxBuffer, &m_rxBuffer[i], sizeof(uint8_t));
        }

        m_rxBuffer[count] = '\0';

        ESP_RESPONSE result = ESP_ParseResponse((char*)m_rxBuffer);

        if (m_onEspResponse != NULL)
        {
            (*m_onEspResponse)(result);
        }
    }
    else
    {
        /* TODO: */
    }
}

void ESP_Init(void)
{
    UartInit(&m_uart, UART_1, BAUD_115200);

    UartRegisterReceiveHandler(&m_uart, &OnUartReceiveCompleted);
}

void ESP_SendCommand(const char* const command)
{
    uint8_t commandLen = strlen(command);

    UartWrite_IT(&m_uart, (uint8_t*)command, commandLen);
    UartWrite_IT(&m_uart, (uint8_t*)"\r\n", 2);
}

void ESP_RegisterResponseHandler(ESP_ResponseHandler_t callback)
{
    m_onEspResponse = callback;
}

static ESP_RESPONSE ESP_ParseResponse(const char* resp)
{
    if (strstr(resp, "OK"))
    {
        return ESP_RESPONSE_OK;
    }
    else if (strstr(resp, "ERROR"))
    {
        return ESP_RESPONSE_ERROR;
    }
    else if (strstr(resp, "BUSY"))
    {
        return ESP_RESPONSE_BUSY;
    }
    else if (strstr(resp, "LED_ON"))
    {
        return ESP_RESPONSE_LED_ON;
    }
    else if (strstr(resp, "LED_OFF"))
    {
        return ESP_RESPONSE_LED_OFF;
    }
    else
    {
        return ESP_RESPONSE_UNKNOWN;
    }
}

