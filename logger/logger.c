#include <assert.h>
#include <string.h>

#include "logger.h"
#include "uart.h"

static Uart_t m_uart;
static LOG_LEVEL m_logLevel = LOG_LEVEL_DEBUG;

const char* const NEW_LINE = "\r\n";
static const char* const PREFIXES[] = { "[DBG]: ", "[INFO]:", "[WARN]: ", "[ERR]: " };

static void NewLine(void)
{
    uint8_t len = strlen(NEW_LINE);
    UartWrite(&m_uart, (uint8_t*)NEW_LINE, len);
}

static void LogGeneric(LOG_LEVEL level, const char* const prefix, const char* const message)
{
    if (m_logLevel < level || m_logLevel == LOG_LEVEL_NONE)
    {
        return;
    }

    uint8_t len = strlen(prefix);
    UartWrite(&m_uart, (uint8_t*)prefix, len);

    len = strlen(message);
    UartWrite(&m_uart, (uint8_t*)message, len);

    NewLine();
}

void LogInit(void)
{
    UartInit(&m_uart, UART_1, BAUD_921600);
}

void LogSetLevel(LOG_LEVEL level)
{
    assert(level < LOG_LEVEL_NUMBER);

    m_logLevel = level;
}

void LogDebug(const char* const message)
{
    LogGeneric(LOG_LEVEL_DEBUG, PREFIXES[LOG_LEVEL_DEBUG], message);
}

void LogInfo(const char* const message)
{
    LogGeneric(LOG_LEVEL_INFO, PREFIXES[LOG_LEVEL_INFO], message);
}

void LogWarn(const char* const message)
{
    LogGeneric(LOG_LEVEL_WARN, PREFIXES[LOG_LEVEL_WARN], message);
}

void LogError(const char* const message)
{
    LogGeneric(LOG_LEVEL_ERROR, PREFIXES[LOG_LEVEL_ERROR], message);
}

void LogHex(uint32_t value)
{
    const char* hex = "0123456789ABCDEF";
    char symbol;
    char hexPrefix[2] = {'0', 'x'};

    UartWrite(&m_uart, (uint8_t*)hexPrefix, sizeof(hexPrefix));

    for (int8_t i = 28; i >= 0; i -= 4)
    {
        symbol = hex[(value >> i) & 0xF];
        if (symbol != '0')
        {
            UartWrite(&m_uart, (uint8_t*)&symbol, sizeof(symbol));
        }
    }

    NewLine();
}

void LogDec(int32_t value)
{
    char buff[12];
    uint8_t i = 0;
    char minus = '-';

    if (value < 0)
    {
        UartWrite(&m_uart, (uint8_t*)&minus, sizeof(minus));
        value = -value;
    }

    do {
        buff[i++] = '0' + (value % 10);
        value /= 10;
    } while (value && i < sizeof(buff));

    while (i--)
    {
        UartWrite(&m_uart, (uint8_t*)&buff[i], sizeof(char));
    }

    NewLine();
}
