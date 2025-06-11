#ifndef LOGGER_H
#define LOGGER_H

typedef enum
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE,
    LOG_LEVEL_NUMBER
} LOG_LEVEL;

void LogInit(void);
void LogSetLevel(LOG_LEVEL level);

void LogDebug(const char* fmt, ...);
void LogInfo(const char* fmt, ...);
void LogWarn(const char* fmt, ...);
void LogError(const char* fmt, ...);

#endif /* LOGGER_H */
