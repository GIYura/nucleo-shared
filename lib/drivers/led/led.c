#include "led.h"

extern const GpioOps_t g_GpioOps;

void LedInit(Led_t* led)
{
    led->gpio->ops = &g_GpioOps;
}

void LedOn(Led_t* led)
{
    led->gpio->ops->write();
}

void LedOff(Led_t* led)
{
    led->gpio->ops->write();
}

void LedToggle(Led_t* led)
{
    led->gpio->ops->toggle();
}

