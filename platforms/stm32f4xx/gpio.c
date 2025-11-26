#include <stddef.h>
#include <stdbool.h>

#include "stm32f411xe.h"

#include "custom-assert.h"
#include "gpio.h"

#define GPIO_IRQ_MAX            (16U)
#define GPIO_PORT_MAX           (8U)

/* Port clock enable */
#define GPIO_CLOCK_ENABLE_PORTA (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN))
#define GPIO_CLOCK_ENABLE_PORTB (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOBEN))
#define GPIO_CLOCK_ENABLE_PORTC (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOCEN))
#define GPIO_CLOCK_ENABLE_PORTD (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIODEN))
#define GPIO_CLOCK_ENABLE_PORTE (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOEEN))
#define GPIO_CLOCK_ENABLE_PORTH (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOHEN))

/* System configuration controller clock enable */
#define SYS_CLOCK_ENABLE        (RCC->APB2ENR |= (RCC_APB2ENR_SYSCFGEN))

static GpioHandle_t* m_GpioIrq[GPIO_IRQ_MAX];

static const GPIO_TypeDef* m_GpioPorts[GPIO_PORT_MAX] = {
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
    GPIOE,
    NULL,
    NULL,
    GPIOH
};

static GPIO_TypeDef* GpioGetPort(PIN_NAMES pinName)
{
    ASSERT(pinName != PIN_NC);

    uint8_t port = (uint8_t)((pinName >> 4U) & 0x0F);

    if (port == 0U) return GPIOA;
    if (port == 1U) return GPIOB;
    if (port == 2U) return GPIOC;
    if (port == 3U) return GPIOD;
    if (port == 4U) return GPIOE;
    if (port == 5U) return GPIOH;

    /* should never reach here */
    return NULL;
}
static uint8_t GpioGetPinIndex(PIN_NAMES pinName)
{
    ASSERT(pinName != PIN_NC);

    return (uint8_t)(pinName & 0x0F);
}

static uint32_t GpioGetExtiLine(const GPIO_TypeDef* const port)
{
    ASSERT(port != NULL);

    for (uint32_t i = 0; i < GPIO_PORT_MAX; i++)
    {
        if (m_GpioPorts[i] == port)
        {
            return i;
        }
    }

    /* should never reach here */
    return 0xFF;
}

static void GpioEnableClocks(const GPIO_TypeDef* const port)
{
    ASSERT(port != NULL);

    if (port == GPIOA)
    {
        GPIO_CLOCK_ENABLE_PORTA;
        return;
    }

    if (port == GPIOB)
    {
        GPIO_CLOCK_ENABLE_PORTB;
        return;
    }

    if (port == GPIOC)
    {
        GPIO_CLOCK_ENABLE_PORTC;
        return;
    }

    if (port == GPIOD)
    {
        GPIO_CLOCK_ENABLE_PORTD;
        return;
    }

    if (port == GPIOE)
    {
        GPIO_CLOCK_ENABLE_PORTE;
        return;
    }

    if (port == GPIOH)
    {
        GPIO_CLOCK_ENABLE_PORTH;
        return;
    }
}

static void GpioSetSpeed(GPIO_TypeDef* port, uint32_t pinIndex, PIN_STRENGTH strength)
{
    ASSERT(port != NULL);

    port->OSPEEDR &= ~(0x03U << (pinIndex * 2U));
    port->OSPEEDR |= ((uint32_t)strength << (pinIndex * 2U));
}

static void GpioSetPull(GPIO_TypeDef* port, uint32_t pinIndex, PIN_TYPES pull)
{
    ASSERT(port != NULL);

    port->PUPDR &= ~(0x03U << (pinIndex * 2U));
    port->PUPDR |= ((uint32_t)pull << (pinIndex * 2U));
}

static void GpioSetMode(GPIO_TypeDef* port, uint32_t pinIndex, PIN_MODES mode)
{
    ASSERT(port != NULL);

    port->MODER &= ~(0x03U << (pinIndex * 2U));
    port->MODER |= ((uint32_t)mode << (pinIndex * 2U));
}

static void GpioSetState(GPIO_TypeDef* port, uint32_t pinIndex, uint32_t value)
{
    ASSERT(port != NULL);
    ASSERT(value == PIN_STATE_LOW || value == PIN_STATE_HIGH);

    if (value == PIN_STATE_LOW)
    {
        port->BSRR = (1U << (pinIndex + 16U));
    }
    else if (value == PIN_STATE_HIGH)
    {
        port->BSRR = (1U << (pinIndex));
    }
}

static void GpioSetAlternateFunction(GPIO_TypeDef* port, uint32_t pinIndex, uint32_t af)
{
    ASSERT(port != NULL);
    ASSERT(af < 16U);

    uint32_t regIndex = (pinIndex < 8U) ? 0U : 1U;
    uint32_t shift = (pinIndex % 8U) * 4U;
    uint32_t mask = 0x0FU << shift;

    port->AFR[regIndex] &= ~mask;
    port->AFR[regIndex] |= ((af & 0x0FU) << shift);
}

static void GpioSetEdge(PIN_IRQ_MODES mode, uint32_t pinIndex)
{
    uint32_t mask = (1U << pinIndex);

    /*clear all */
    EXTI->RTSR &= ~mask;
    EXTI->FTSR &= ~mask;

    switch (mode)
    {
        case PIN_IRQ_RISING:
            EXTI->RTSR |= mask;
            break;

        case PIN_IRQ_FALLING:
            EXTI->FTSR |= mask;
            break;

        case PIN_IRQ_BOTH:
            EXTI->RTSR |= mask;
            EXTI->FTSR |= mask;
            break;

        default:
            /* should never reach here */
            ASSERT(false);
            break;
    }
}

static IRQn_Type GpioGetIrqNumber(uint32_t pinIndex)
{
    ASSERT(pinIndex < 16U);

    if (pinIndex <= 4)
    {
        return (IRQn_Type)(EXTI0_IRQn + pinIndex);
    }
    else if (pinIndex <= 9)
    {
        return EXTI9_5_IRQn;
    }
    else
    {
        return EXTI15_10_IRQn;
    }
}

static void GpioExtiHandler(uint8_t first, uint8_t last)
{
    for (uint8_t i = first; i <= last; i++)
    {
        uint32_t mask = (1U << i);

        if (EXTI->PR & mask)
        {
            /*
             * (rc_w1) Software can read as well as clear this bit by writing 1.
             * Writing ‘0’ has no effect on the bit value.
             * */
            EXTI->PR = mask;

            if (m_GpioIrq[i] != NULL && m_GpioIrq[i]->irqHandler != NULL)
            {
                (*m_GpioIrq[i]->irqHandler)();
            }
        }
    }
}

void GpioInit(GpioHandle_t* handle,
              PIN_NAMES pinName,
              PIN_MODES mode,
              PIN_TYPES pull,
              PIN_STRENGTH strength,
              PIN_CONFIGS config,
              uint32_t value)
{
    ASSERT(handle != NULL);

    if (pinName == PIN_NC)
    {
        return;
    }

    GPIO_TypeDef* port = GpioGetPort(pinName);
    uint8_t pinIndex = GpioGetPinIndex(pinName);

    ASSERT(port != NULL);

    handle->hw.stm32f411.port = port;
    handle->hw.stm32f411.pinIndex = pinIndex;

    GpioEnableClocks(port);
    GpioSetSpeed(port, pinIndex, strength);
    GpioSetPull(port, pinIndex, pull);
    GpioSetMode(port, pinIndex, mode);

    /* 0: Output push-pull (reset state) */
    port->OTYPER &= ~(1U << pinIndex);

    if (config == PIN_CONFIG_OPEN_DRAIN)
    {
        /* 1: Output open-drain */
        port->OTYPER |= (1U << pinIndex);
    }

    if (mode == PIN_MODE_ALTERNATE)
    {
        GpioSetAlternateFunction(port, pinIndex, value);
        return;
    }

    if (mode == PIN_MODE_OUTPUT)
    {
        GpioSetState(port, pinIndex, value);
    }
}

void GpioWrite(const GpioHandle_t* const handle, PIN_STATES state)
{
    ASSERT(handle != NULL);

    GPIO_TypeDef* port = (GPIO_TypeDef*)handle->hw.stm32f411.port;
    uint8_t pinIndex = handle->hw.stm32f411.pinIndex;

    GpioSetState(port, pinIndex, state);
}

uint16_t GpioRead(const GpioHandle_t* const handle)
{
    ASSERT(handle != NULL);

    GPIO_TypeDef* port = (GPIO_TypeDef*)handle->hw.stm32f411.port;
    uint8_t pinIndex = handle->hw.stm32f411.pinIndex;

    uint16_t value = (uint16_t)(port->IDR & (1U << pinIndex));

    value &= (1U<< (pinIndex));

    return value ? 1U : 0U;
}

void GpioToggle(const GpioHandle_t* const handle)
{
    ASSERT(handle != NULL);

    GPIO_TypeDef* port = (GPIO_TypeDef*)handle->hw.stm32f411.port;
    uint8_t pinIndex = handle->hw.stm32f411.pinIndex;

    uint32_t odr = port->ODR;

    if (odr & (1 << pinIndex))
    {
        GpioSetState(port, pinIndex, PIN_STATE_LOW);
    }
    else
    {
        GpioSetState(port, pinIndex, PIN_STATE_HIGH);
    }
}

void GpioSetInterrupt(GpioHandle_t* const handle, PIN_IRQ_MODES mode, uint8_t priority, GpioIrqHandler handler)
{
    ASSERT(handle != NULL);
    ASSERT(handler != NULL);

    GPIO_TypeDef* port = (GPIO_TypeDef*)handle->hw.stm32f411.port;
    uint8_t pinIndex = handle->hw.stm32f411.pinIndex;
    uint32_t mask = (1U << pinIndex);

    for (uint8_t i = 0; i < GPIO_IRQ_MAX; i++)
    {
        if (m_GpioIrq[i] != NULL && m_GpioIrq[i]->hw.stm32f411.pinIndex == pinIndex)
        {
            return;
        }
    }

    handle->irqHandler = handler;

    SYS_CLOCK_ENABLE;

    uint8_t extiReg = (pinIndex / 4);
    uint8_t extiIndex = (pinIndex % 4) * 4;
    uint8_t extiValue = GpioGetExtiLine(port);
    IRQn_Type irqNum = GpioGetIrqNumber(pinIndex);

    ASSERT(extiValue != 0xff);

    /* assign lines EXTIx to ports PA..PH */
    SYSCFG->EXTICR[extiReg] &= ~(0x0F << extiIndex);
    SYSCFG->EXTICR[extiReg] |= (extiValue << extiIndex);

    /* disable line on IMR */
    EXTI->IMR &= ~mask;

    GpioSetEdge(mode, pinIndex);

    /* clear pending on EXTI */
    if (EXTI->PR & mask)
    {
        EXTI->PR = mask;
    }

    NVIC_ClearPendingIRQ(irqNum);

    m_GpioIrq[pinIndex] = handle;

    NVIC_SetPriority(irqNum, priority);
    NVIC_EnableIRQ(irqNum);

    /* enable line on IMR */
    EXTI->IMR |= mask;
}

void EXTI0_IRQHandler(void)
{
    GpioExtiHandler(0, 0);
}

void EXTI1_IRQHandler(void)
{
    GpioExtiHandler(1, 1);
}

void EXTI2_IRQHandler(void)
{
    GpioExtiHandler(2, 2);
}

void EXTI3_IRQHandler(void)
{
    GpioExtiHandler(3, 3);
}

void EXTI4_IRQHandler(void)
{
    GpioExtiHandler(4, 4);
}

void EXTI9_5_IRQHandler(void)
{
    GpioExtiHandler(5, 9);
}

void EXTI15_10_IRQHandler(void)
{
    GpioExtiHandler(10, 15);
}
