#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#include "logger.h"

static LOG_LEVEL m_logLevel = LOG_LEVEL_DEBUG;

static void LogGeneric(LOG_LEVEL level, const char* prefix, const char* fmt, va_list args);

void LogInit(void)
{

}

void LogSetLevel(LOG_LEVEL level)
{
    assert(level < LOG_LEVEL_NUMBER);

    m_logLevel = level;
}

#define DEFINE_LOG_FUNC(name, level, prefix)    \
    void Log##name(const char* fmt, ...)        \
    {                                           \
        va_list args;                           \
        va_start(args, fmt);                    \
        LogGeneric(level, prefix, fmt, args);   \
        va_end(args);                           \
    }                                           \

DEFINE_LOG_FUNC(Debug, LOG_LEVEL_DEBUG, "DBG")
DEFINE_LOG_FUNC(Info, LOG_LEVEL_INFO, "INF")
DEFINE_LOG_FUNC(Warn, LOG_LEVEL_WARN, "WRN")
DEFINE_LOG_FUNC(Error, LOG_LEVEL_ERROR, "ERR")

static void LogGeneric(LOG_LEVEL level, const char* prefix, const char* fmt, va_list args)
{
    if (level < m_logLevel)
    {
        return;
    }
#if 0
    char buffer[128];
    int n = snprintf(buffer, sizeof(buffer), "[%s] ", prefix);
    vsnprintf(&buffer[n], sizeof(buffer) - n, fmt, args);
#endif
}
