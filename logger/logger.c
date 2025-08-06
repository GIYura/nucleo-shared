#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include "logger.h"
#include "uart.h"

static Uart_t m_uart;
static LOG_LEVEL m_logLevel = LOG_LEVEL_DEBUG;
static bool m_isIdle = false;

const char* const NEW_LINE = "\r\n";
static const char* const PREFIXES[] = { "[DBG]: ", "[INFO]:", "[WARN]: ", "[ERR]: " };

static void PrintPrefix(const char* const message);
static void PrintMessage(const char* const message);
static void PrintChar(char ch);
static void PrintHex(uint32_t value);
static void PrintDec(int32_t value);
static void PrintNewLine(void);

static void PrintPrefix(const char* const message)
{
    uint8_t len = strlen(PREFIXES[m_logLevel]);
    UartWrite(&m_uart, (uint8_t*)PREFIXES[m_logLevel], len);
}

static void PrintMessage(const char* const message)
{
    PrintPrefix(PREFIXES[m_logLevel]);

    uint8_t len = strlen(message);
    UartWrite(&m_uart, (uint8_t*)message, len);
}

static void PrintChar(char ch)
{
    UartWrite(&m_uart, (uint8_t*)&ch, 1);
}

static void PrintHex(uint32_t value)
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
}

static void PrintDec(int32_t value)
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
}

static void PrintNewLine(void)
{
    uint8_t len = strlen(NEW_LINE);
    UartWrite(&m_uart, (uint8_t*)NEW_LINE, len);
}

void LogInit(LOG_LEVEL level)
{
    assert(level < LOG_LEVEL_NUMBER);

    UartInit(&m_uart, UART_1, BAUD_921600);

    m_logLevel = level;
}

void LogPrint(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while (*fmt)
    {
        if (*fmt == '%')
        {
            fmt++;
            switch (*fmt)
            {
                case 'd':
                {
                    int val = va_arg(args, int);
                    PrintDec(val);
                    break;
                }

                case 'u':
                {
                    uint32_t val = va_arg(args, uint32_t);
                    PrintDec((int32_t)val);
                    break;
                }

                case 'x':
                {
                    int val = va_arg(args, uint32_t);
                    PrintHex(val);
                    break;
                }

                case 's':
                {
                    char* str = va_arg(args, char*);
                    PrintMessage(str);
                    break;
                }

                case 'c':
                {
                    char ch = va_arg(args, int);
                    PrintChar(ch);
                    break;
                }

                case '%':
                {
                    PrintChar('%');
                    break;
                }

                default:
                {
                    PrintChar('?');
                    break;
                }
            }
        }
        else
        {
            PrintChar(*fmt);
        }

        fmt++;
    }

    PrintNewLine();

    va_end(args);
}

void LogFlush(void)
{
    while (!UartIdle(&m_uart));

    m_isIdle = true;
}

bool LogIdle(void)
{
    return m_isIdle;
}

