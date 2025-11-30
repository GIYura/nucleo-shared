#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

#define PIN_NC                  (-1)

/* pin direction / mode */
typedef enum
{
    PIN_MODE_INPUT = 0,
    PIN_MODE_OUTPUT,
    PIN_MODE_ALTERNATE,
    PIN_MODE_ANALOG
} PIN_MODES;

/* pull-up/pull-down */
typedef enum
{
    PIN_TYPE_NO_PULL = 0,
    PIN_TYPE_PULL_UP,
    PIN_TYPE_PULL_DOWN
} PIN_TYPES;

/* push-pull / open-drain */
typedef enum
{
    PIN_CONFIG_PUSH_PULL = 0,
    PIN_CONFIG_OPEN_DRAIN
} PIN_CONFIGS;

/* speed / drive strength */
typedef enum
{
    PIN_STRENGTH_LOW = 0,
    PIN_STRENGTH_MEDIUM,
    PIN_STRENGTH_HIGH
} PIN_STRENGTH;

/* pin states */
typedef enum
{
    PIN_STATE_LOW = 0,
    PIN_STATE_HIGH
} PIN_STATES;

/* IRQ mode */
typedef enum
{
    PIN_IRQ_NONE = 0,
    PIN_IRQ_RISING,
    PIN_IRQ_FALLING,
    PIN_IRQ_BOTH
} PIN_IRQ_MODES;

typedef void (*GpioIrqHandler)(void);

typedef struct
{
    GpioIrqHandler irqHandler;

    union
    {
        struct
        {
            void* port;
            uint8_t pinIndex;
        } stm32f411;

        struct
        {
            uint32_t base;
            uint8_t pinMask;
            uint8_t pinIndex;
        } cc3220;

        struct
        {
            volatile uint8_t* ddr;
            volatile uint8_t* port;
            volatile uint8_t* pinReg;
            uint8_t bit;
        } atmega328;
    } hw;
} GpioHandle_t;

/*Brief: Gpio initialization
 * [in] - handle - pointer to gpio object
 * [in] - pin - name of the pin defined in platforms/gpio-name.h
 * [in] - mode - gpio mode
 * [in] - pull - gpio pull-up/pull-down
 * [in] - strength - gpio speed
 * [in] - config - gpio config
 * [in] - value - gpio default value
 * NOTE: this param can be used as alternate function
 * [out] - none
 * */
void GpioInit(GpioHandle_t* const handle,
              uint8_t pin,
              PIN_MODES mode,
              PIN_TYPES pull,
              PIN_STRENGTH strength,
              PIN_CONFIGS config,
              uint32_t value);

/*Brief: Gpio write
 * [in] - handle - pointer to gpio object
 * [in] - value - new gpio value
 * [out] - none
 * */
void GpioWrite(const GpioHandle_t* const handle, PIN_STATES state);

/*Brief: Gpio read
 * [in] - handle - pointer to gpio object
 * [out] - value - gpio state
 * */
uint16_t GpioRead(const GpioHandle_t* const handle);

/*Brief: Gpio toggle
 * [in] - handle - pointer to gpio object
 * [out] - none
 * */
void GpioToggle(const GpioHandle_t* const handle);

/*Brief: Gpio IRQ initialization
 * [in] - handle - pointer to gpio object
 * [in] - mode - IRQ mode
 * [in] - priority - IRQ priority
 * [in] - handler - callback function pointer
 * [out] - none
 * */
void GpioSetInterrupt(GpioHandle_t* const handle, PIN_IRQ_MODES mode, uint8_t priority, GpioIrqHandler handler);

#endif /* GPIO_H */
