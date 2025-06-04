#ifndef GPIO_H
#define GPIO_H

#include "stm32f411xe.h"
#include "gpio-name.h"

/* Port names */
#define PORTA   GPIOA
#define PORTB   GPIOB
#define PORTC   GPIOC
#define PORTD   GPIOD
#define PORTE   GPIOE
#define PORTH   GPIOH

/* Port clock enable */
#define GPIO_CLOCK_ENABLE_PORTA (RCC->AHB1ENR |= (1 << 0))
#define GPIO_CLOCK_ENABLE_PORTB (RCC->AHB1ENR |= (1 << 1))
#define GPIO_CLOCK_ENABLE_PORTC (RCC->AHB1ENR |= (1 << 2))
#define GPIO_CLOCK_ENABLE_PORTD (RCC->AHB1ENR |= (1 << 3))
#define GPIO_CLOCK_ENABLE_PORTE (RCC->AHB1ENR |= (1 << 4))
#define GPIO_CLOCK_ENABLE_PORTH (RCC->AHB1ENR |= (1 << 7))

/* Alternate gpio */
typedef enum
{
    PIN_AF_0 = 0,
    PIN_AF_1,
    PIN_AF_2,
    PIN_AF_3,
    PIN_AF_4,
    PIN_AF_5,
    PIN_AF_6,
    PIN_AF_7,
    PIN_AF_8,
    PIN_AF_9,
    PIN_AF_10,
    PIN_AF_11,
    PIN_AF_12,
    PIN_AF_13,
    PIN_AF_14,
    PIN_AF_15,
} PIN_AF;

/* Pin states */
typedef enum
{
    PIN_STATE_LOW = 0,
    PIN_STATE_HIGH
} PIN_STATES;

/* Pin modes */
typedef enum
{
    PIN_MODE_INPUT = 0,
    PIN_MODE_OUTPUT,
    PIN_MODE_ALTERNATE,
    PIN_MODE_ANALOG,
} PIN_MODES;

/* Pin pull-up/pull-down */
typedef enum
{
    PIN_CONFIG_PUSH_PULL = 0,
    PIN_CONFIG_OPEN_DRAIN,
} PIN_CONFIGS;

/* Pin type */
typedef enum
{
    PIN_TYPE_NO_PULL_UP_PULL_DOWN = 0,
    PIN_TYPE_PULL_UP,
    PIN_TYPE_PULL_DOWN,
} PIN_TYPES;

/* Pin speed */
typedef enum
{
    PIN_SPEED_LOW = 0,
    PIN_SPEED_MEDIUM,
    PIN_SPEED_FAST,
    PIN_SPEED_HIGH,
} PIN_SPEEDS;

/* pin IRQ mode */
typedef enum
{
    PIN_IRQ_NO = 0,
    PIN_IRQ_RISING,
    PIN_IRQ_FALING,
    PIN_IRQ_RISING_FALING,
} PIN_IRQ_MODES;

/* pin IRQ priority */
typedef enum
{
    PIN_IRQ_PRIORITY_HIGH = 3,
    PIN_IRQ_PRIORITY_MEDIUM,
    PIN_IRQ_PRIORITY_LOW,
} PIN_IRQ_PRIORITIES;

typedef void (GpioIrqHandler)(void);

typedef struct
{
    GPIO_TypeDef* port;
    PIN_NAMES pinName;
    uint8_t pinIndex;
    PIN_MODES mode;
    PIN_TYPES type;
    PIN_SPEEDS speed;
    PIN_CONFIGS config;
    GpioIrqHandler* irqHandler;
} Gpio_t;

/*Brief: Gpio initialization
 * [in] - obj - pointer to gpio object
 * [in] - pinName - name of the pin defined in gpio-name.h
 * [in] - mode - gpio mode
 * [in] - type - gpio type
 * [in] - speed - gpio speed
 * [in] - config - gpio config
 * [in] - value - gpio default value
 * [out] - none
 * */
void GpioInit(  Gpio_t* const obj,
                PIN_NAMES pinName,
                PIN_MODES mode,
                PIN_TYPES type,
                PIN_SPEEDS speed,
                PIN_CONFIGS config,
                uint32_t value);

/*Brief: Gpio write
 * [in] - obj - pointer to gpio object
 * [in] - value - new gpio value
 * [out] - none
 * */
void GpioWrite(const Gpio_t* const obj, uint32_t value);

/*Brief: Gpio read
 * [in] - obj - pointer to gpio object
 * [out] - value - gpio state
 * */
uint32_t GpioRead(const Gpio_t* const obj);

/*Brief: Gpio toggle
 * [in] - obj - pointer to gpio object
 * [out] - none
 * */
void GpioToggle(const Gpio_t* const obj);

/*Brief: Gpio IRQ initialization
 * [in] - irqMode - IRQ mode
 * [in] - irqPriority - IRQ priority
 * [in] - handler - callback function pointer
 * [out] - none
 * */
void GpioSetInterrupt(Gpio_t* obj, PIN_IRQ_MODES irqMode, PIN_IRQ_PRIORITIES irqPriority, GpioIrqHandler* const handler);

#endif /* GPIO_H */
