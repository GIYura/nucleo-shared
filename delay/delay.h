#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

/*Brief: Blocking delay in seconds
 * [in] - seconds
 * [out] - none
 * */
void DelaySec(uint32_t sec);

/*Brief: Blocking delay in milliseconds
 * [in] - milliseconds
 * [out] - none
 * */
void DelayMs(uint32_t msec);

/*Brief: Blocking delay in microseconds
 * [in] - microseconds
 * [out] - none
 * */
void DelayUs(uint32_t usec);

#endif /* DELAY_H */

