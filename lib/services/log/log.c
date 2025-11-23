#include <string.h>
#include <stdarg.h>

#include "assert.h"
#include "log.h"
#include "uart-service.h"

static LOG_LEVEL m_logLevel = LOG_LEVEL_DEBUG;

static const char* const PREFIXES[LOG_LEVEL_NUMBER] = { "[DBG]: ", "[INFO]:", "[WARN]: ", "[ERR]: ", "" };

static void PrintPrefix(const char* const message, uint8_t len);
static void PrintMessage(const char* const message);
static void PrintChar(char ch);
static void PrintHex(uint32_t value);
static void PrintDec(int32_t value);
static void PrintNewLine(void);

static void PrintPrefix(const char* const message, uint8_t len)
{
    UartServiceSend((uint8_t*)PREFIXES[m_logLevel], len);
}

static void PrintMessage(const char* const message)
{
    uint8_t len = strlen(PREFIXES[m_logLevel]);

    if (len != 0)
    {
        PrintPrefix(PREFIXES[m_logLevel], len);
    }

    len = strlen(message);

    UartServiceSend((uint8_t*)message, len);
}

static void PrintChar(char ch)
{
    UartServiceSend((uint8_t*)&ch, sizeof(char));
}

static void PrintHex(uint32_t value)
{
    const char* const HEX = "0123456789ABCDEF";
    char symbol = 0;
    char hexPrefix[2] = {'0', 'x'};
    char zeros[2] = {'0', '0'};
    uint8_t index = 0;

    UartServiceSend((uint8_t*)hexPrefix, sizeof(hexPrefix));

    if (value == 0)
    {
        UartServiceSend((uint8_t*)zeros, sizeof(zeros));
    }

    for (int8_t i = 28; i >= 0; i -= 4)
    {
        index = (value >> i) & 0x0F;
        symbol = HEX[index];

        if (symbol != '0')
        {
            UartServiceSend((uint8_t*)&symbol, sizeof(symbol));
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
        UartServiceSend((uint8_t*)&minus, sizeof(minus));
        value = -value;
    }

    do {
        buff[i++] = '0' + (value % 10);
        value /= 10;
    } while (value && i < sizeof(buff));

    while (i--)
    {
        UartServiceSend((uint8_t*)&buff[i], sizeof(char));
    }
}

static void PrintNewLine(void)
{
    const char* const NEW_LINE = "\r\n";

    uint8_t len = strlen(NEW_LINE);

    UartServiceSend((uint8_t*)NEW_LINE, len);
}

void LogLevel(LOG_LEVEL level)
{
    ASSERT(level < LOG_LEVEL_NUMBER);

    m_logLevel = level;
}

void LogPrint(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if (m_logLevel == LOG_LEVEL_NONE)
    {
        return;
    }

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

                case '\r':
                case '\n':
                    PrintNewLine();
                    break;

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

    va_end(args);
}

bool LogIdle(void)
{
    return UartServiceIdle();
}

