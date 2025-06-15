#include <assert.h>
#include <stddef.h>

#include "gpio.h"

#define INTERRUPT_MAX    16

static Gpio_t* m_GpioIrq[INTERRUPT_MAX];

void GpioInit(  Gpio_t* const obj,
                PIN_NAMES pinName,
                PIN_MODES mode,
                PIN_TYPES type,
                PIN_SPEEDS speed,
                PIN_CONFIGS config,
                uint32_t value)
{
    assert(obj != NULL);

    if (pinName == NC)
    {
        return;
    }

    obj->pinName = pinName;
    obj->pinIndex = (obj->pinName & 0x0F);

    if ((obj->pinName & 0xF0) == 0x00)
    {
        obj->port = GPIOA;
        GPIO_CLOCK_ENABLE_PORTA;
    }
    else if ((obj->pinName & 0xF0) == 0x10)
    {
        obj->port = GPIOB;
        GPIO_CLOCK_ENABLE_PORTB;
    }
    else if ((obj->pinName & 0xF0) == 0x20)
    {
        obj->port = GPIOC;
        GPIO_CLOCK_ENABLE_PORTC;
    }
    else if ((obj->pinName & 0xF0) == 0x30)
    {
        obj->port = GPIOD;
        GPIO_CLOCK_ENABLE_PORTD;
    }
    else if ((obj->pinName & 0xF0) == 0x40)
    {
        obj->port = GPIOE;
        GPIO_CLOCK_ENABLE_PORTE;
    }
    else if ((obj->pinName & 0xF0) == 0x50)
    {
        obj->port = GPIOH;
        GPIO_CLOCK_ENABLE_PORTH;
    }
    else
    {
        assert(0);
    }

    obj->port->OSPEEDR &= ~(0x03 << (obj->pinIndex * 2));
    obj->port->OSPEEDR |= (speed << (obj->pinIndex * 2));

    obj->port->PUPDR &= ~(0x03 << (obj->pinIndex * 2));
    obj->port->PUPDR |= (type << (obj->pinIndex * 2));

    obj->port->MODER &= ~(0x03 << (obj->pinIndex * 2));
    obj->port->MODER |= (mode << (obj->pinIndex * 2));

    if (mode == PIN_MODE_ALTERNATE)
    {
        if (obj->pinIndex < 8)
        {
            obj->port->AFR[0] |= (value << (obj->pinIndex * 4));
        }
        else
        {
            obj->port->AFR[1] |= (value << ((obj->pinIndex % 8) * 4));
        }
    }

    if (mode == PIN_MODE_OUTPUT)
    {
        obj->port->OTYPER |= (config << (obj->pinIndex));

        if (value == PIN_STATE_LOW)
        {
            obj->port->BSRR = (1 << (obj->pinIndex + 16));
        }
        else
        {
            obj->port->BSRR = (1 << (obj->pinIndex));
        }
    }
}

void GpioWrite(const Gpio_t* const obj, uint32_t value)
{
    assert(obj != NULL);

    if (obj->pinName == NC)
    {
        return;
    }

    if (value)
    {
        obj->port->BSRR = (1 << (obj->pinIndex));
    }
    else
    {
        obj->port->BSRR = (1 << (obj->pinIndex + 16));
    }
}

uint32_t GpioRead(const Gpio_t* const obj)
{
    assert(obj != NULL);

    uint16_t value = obj->port->IDR;

    value &= (1 << (obj->pinIndex));

    return value ? 1 : 0;
}

void GpioToggle(const Gpio_t* const obj)
{
    assert(obj != NULL);

    uint32_t odr = (obj->port->ODR);

    if (odr & (1 << obj->pinIndex))
    {
        obj->port->BSRR = (1 << (obj->pinIndex + 16));
    }
    else
    {
        obj->port->BSRR = (1 << (obj->pinIndex));
    }
}

void GpioSetInterrupt(Gpio_t* obj, PIN_IRQ_MODES irqMode, PIN_IRQ_PRIORITIES irqPriority, GpioIrqHandler* const handler)
{
    assert(obj != NULL);
    assert(handler != NULL);

    if (obj->pinName == NC)
    {
        return;
    }

    for (uint8_t i = 0; i < INTERRUPT_MAX; i++)
    {
        if (m_GpioIrq[i] != NULL && m_GpioIrq[i]->pinIndex == obj->pinIndex)
        {
            return;
        }
    }

    obj->irqHandler = handler;

    /* System configuration controller clock enable */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    uint8_t extiReg = (obj->pinIndex / 4);
    uint8_t extiIndex = (obj->pinIndex % 4) * 4;
    uint8_t extiValue = 0x00;
    IRQn_Type irqNum = EXTI0_IRQn;

    if ((obj->pinName & 0xF0) == 0x00)
    {
        extiValue = 0x00;
    }
    else if ((obj->pinName & 0xF0) == 0x10)
    {
        extiValue = 0x01;
    }
    else if ((obj->pinName & 0xF0) == 0x20)
    {
        extiValue = 0x02;
    }
    else if ((obj->pinName & 0xF0) == 0x30)
    {
        extiValue = 0x03;
    }
    else if ((obj->pinName & 0xF0) == 0x40)
    {
        extiValue = 0x04;
    }
    else if ((obj->pinName & 0xF0) == 0x50)
    {
        extiValue = 0x07;
    }

    /* clear and set */
    SYSCFG->EXTICR[extiReg] &= ~(0x0F << extiIndex);
    SYSCFG->EXTICR[extiReg] |= (extiValue << extiIndex);

    /* enable interrupt */
    EXTI->IMR |= (1 << obj->pinIndex);

    /* edge setup */
    switch (irqMode)
    {
        case PIN_IRQ_RISING:
            EXTI->RTSR |= (1 << obj->pinIndex);
            break;

        case PIN_IRQ_FALING:
            EXTI->FTSR |= (1 << obj->pinIndex);
            break;

        default:
            EXTI->RTSR |= (1 << obj->pinIndex);
            EXTI->FTSR |= (1 << obj->pinIndex);
            break;
    }

    if (obj->pinIndex <= 4)
    {
        irqNum += obj->pinIndex;
    }
    else if (obj->pinIndex <= 9)
    {
        irqNum = EXTI9_5_IRQn;
    }
    else
    {
        irqNum = EXTI15_10_IRQn;
    }

    m_GpioIrq[obj->pinIndex] = obj;

    if (NVIC_GetPendingIRQ(irqNum) != 0)
    {
        EXTI->PR |= (1 << obj->pinIndex);
        NVIC_ClearPendingIRQ(irqNum);
    }

    NVIC_SetPriority(irqNum, irqPriority);
    NVIC_EnableIRQ(irqNum);
}

void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & (1 << 0))
    {
        EXTI->PR |= (1 << 0);

        if (m_GpioIrq[0]->irqHandler != NULL)
        {
            (*m_GpioIrq[0]->irqHandler)();
        }
    }
}

void EXTI1_IRQHandler(void)
{
    if (EXTI->PR & (1 << 1))
    {
        EXTI->PR |= (1 << 1);

        if (m_GpioIrq[1]->irqHandler != NULL)
        {
            (*m_GpioIrq[1]->irqHandler)();
        }
    }
}

void EXTI2_IRQHandler(void)
{
    if (EXTI->PR & (1 << 2))
    {
        EXTI->PR |= (1 << 2);

        if (m_GpioIrq[2]->irqHandler != NULL)
        {
            (*m_GpioIrq[2]->irqHandler)();
        }
    }
}

void EXTI3_IRQHandler(void)
{
    if (EXTI->PR & (1 << 3))
    {
        EXTI->PR |= (1 << 3);

        if (m_GpioIrq[3]->irqHandler != NULL)
        {
            (*m_GpioIrq[3]->irqHandler)();
        }
    }
}

void EXTI4_IRQHandler(void)
{
    if (EXTI->PR & (1 << 4))
    {
        EXTI->PR |= (1 << 4);

        if (m_GpioIrq[4]->irqHandler != NULL)
        {
            (*m_GpioIrq[4]->irqHandler)();
        }
    }
}

void EXTI9_5_IRQHandler(void)
{
    for (uint8_t i = 5; i <= 9; i++)
    {
        if (EXTI->PR & (1 << i))
        {
            EXTI->PR |= (1 << i);

            if (m_GpioIrq[i]->irqHandler != NULL)
            {
                (*m_GpioIrq[i]->irqHandler)();
            }
        }
    }
}

void EXTI15_10_IRQHandler(void)
{
    for (uint8_t i = 10; i <= 15; i++)
    {
        if (EXTI->PR & (1 << i))
        {
            EXTI->PR |= (1 << i);

            if (m_GpioIrq[i]->irqHandler != NULL)
            {
                (*m_GpioIrq[i]->irqHandler)();
            }
        }
    }
}

