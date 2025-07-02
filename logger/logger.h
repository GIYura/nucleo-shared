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

/*Brief: Logger initialization
 * [in] - level - logger level
 * [out] - none
 * */
void LogInit(LOG_LEVEL level);

/*Brief: Send message
 * [in] - fmt - supported formats: %u %d %x %s %c
 * [out] - none
 * */
void LogPrint(const char *fmt, ...);

#endif /* LOGGER_H */
