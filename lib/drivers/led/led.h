#ifndef LED_H
#define LED_H

#include "gpio.h"

typedef struct
{
    GpioHandle_t* gpio;
} Led_t;

void LedInit(Led_t* led);
void LedOn(Led_t* led);
void LedOff(Led_t* led);
void LedToggle(Led_t* led);

#endif /* LED_H */
