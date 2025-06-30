#include <assert.h>
#include <string.h>

#include "logger.h"
#include "uart.h"

static Uart_t m_uart;
static LOG_LEVEL m_logLevel = LOG_LEVEL_DEBUG;

static const char* const PREFIX[] = { "[DBG]: ", "[INFO]:", "[WARN]: ", "[ERR]: " };

static void LogGeneric(LOG_LEVEL level, const char* const prefix, const char* const message)
{
    if (m_logLevel < level || m_logLevel == LOG_LEVEL_NONE)
    {
        return;
    }
    const char* const newLine = "\r\n";

    uint8_t len = strlen(prefix);
    UartWrite(&m_uart, (uint8_t*)prefix, len);

    len = strlen(message);
    UartWrite(&m_uart, (uint8_t*)message, len);

    len = strlen(newLine);
    UartWrite(&m_uart, (uint8_t*)newLine, len);
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
    LogGeneric(LOG_LEVEL_DEBUG, PREFIX[LOG_LEVEL_DEBUG], message);
}

void LogInfo(const char* const message)
{
    LogGeneric(LOG_LEVEL_INFO, PREFIX[LOG_LEVEL_INFO], message);
}

void LogWarn(const char* const message)
{
    LogGeneric(LOG_LEVEL_WARN, PREFIX[LOG_LEVEL_WARN], message);
}

void LogError(const char* const message)
{
    LogGeneric(LOG_LEVEL_ERROR, PREFIX[LOG_LEVEL_ERROR], message);
}
