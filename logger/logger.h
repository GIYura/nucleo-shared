#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdbool.h>

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
 * [in] - none
 * [out] - none
 * */
void LogInit(void);

/*Brief: Set logger level
 * [in] - level - new level
 * [out] - none
 * */
void LogLevel(LOG_LEVEL level);

/*Brief: Send message
 * [in] - fmt - supported formats: %u %d %x %s %c
 * [out] - none
 * */
void LogPrint(const char *fmt, ...);

/*Brief: Check if logger is Idle
 * [in] - none
 * [out] - true - idle; false - otherwise (still transmitting)
 * */
bool LogIdle(void);

#endif /* LOGGER_H */
