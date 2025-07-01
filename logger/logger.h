#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>

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

void LogDebug(const char* const message);
void LogInfo(const char* const message);
void LogWarn(const char* const message);
void LogError(const char* const message);

void LogHex(uint32_t value);
void LogDec(int32_t value);

#endif /* LOGGER_H */
