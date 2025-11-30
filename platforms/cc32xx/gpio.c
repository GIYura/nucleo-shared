#include <stddef.h>

#include "custom-assert.h"

#include "gpio.h"
#include "gpio-name.h"

#include <ti/devices/cc32xx/inc/hw_types.h>
#include <ti/devices/cc32xx/inc/hw_memmap.h>

#include <ti/devices/cc32xx/driverlib/gpio.h>
#include <ti/devices/cc32xx/driverlib/pin.h>
#include <ti/devices/cc32xx/driverlib/prcm.h>
#include <ti/devices/cc32xx/driverlib/interrupt.h>

static GpioHandle_t* m_GpioIrq[PIN_GPIO_MAX] = {0};

typedef struct
{
    uint32_t base;      /* GPIOAx_BASE [x = 0 - 3] */
    uint8_t mask;       /* GPIO_PIN_x [x = 0 - 7] */
    uint8_t pin;        /* PIN_xx (x = 0 - 63) defined in driverlib/pin.h */
} GpioMap_t;

static const GpioMap_t m_GPIO_MAP[PIN_GPIO_MAX] = {
        [PIN_GPIOA0_0] = { GPIOA0_BASE, GPIO_PIN_0, PIN_50 },
        [PIN_GPIOA0_1] = { GPIOA0_BASE, GPIO_PIN_1, PIN_55 },
        [PIN_GPIOA0_2] = { GPIOA0_BASE, GPIO_PIN_2, PIN_57 },
        [PIN_GPIOA0_3] = { GPIOA0_BASE, GPIO_PIN_3, PIN_58 },
        [PIN_GPIOA0_4] = { GPIOA0_BASE, GPIO_PIN_4, PIN_59 },
        [PIN_GPIOA0_5] = { GPIOA0_BASE, GPIO_PIN_5, PIN_60 },
        [PIN_GPIOA0_6] = { GPIOA0_BASE, GPIO_PIN_6, PIN_61 },
        [PIN_GPIOA0_7] = { GPIOA0_BASE, GPIO_PIN_7, PIN_62 },

        [PIN_GPIOA1_0] = { GPIOA1_BASE, GPIO_PIN_0, PIN_63 },
        [PIN_GPIOA1_1] = { GPIOA1_BASE, GPIO_PIN_1, PIN_64 },
        [PIN_GPIOA1_2] = { GPIOA1_BASE, GPIO_PIN_2, PIN_01 },
        [PIN_GPIOA1_3] = { GPIOA1_BASE, GPIO_PIN_3, PIN_02 },
        [PIN_GPIOA1_4] = { GPIOA1_BASE, GPIO_PIN_4, PIN_03 },
        [PIN_GPIOA1_5] = { GPIOA1_BASE, GPIO_PIN_5, PIN_04 },
        [PIN_GPIOA1_6] = { GPIOA1_BASE, GPIO_PIN_6, PIN_05 },
        [PIN_GPIOA1_7] = { GPIOA1_BASE, GPIO_PIN_7, PIN_06 },

        [PIN_GPIOA2_0] = { GPIOA2_BASE, GPIO_PIN_0, PIN_07 },
        [PIN_GPIOA2_1] = { GPIOA2_BASE, GPIO_PIN_1, PIN_08 },

       [PIN_GPIOA2_6] = { GPIOA2_BASE, GPIO_PIN_6, PIN_15 },
       [PIN_GPIOA2_7] = { GPIOA2_BASE, GPIO_PIN_7, PIN_16 },

       [PIN_GPIOA3_0] = { GPIOA3_BASE, GPIO_PIN_0, PIN_17 },
       [PIN_GPIOA3_1] = { GPIOA3_BASE, GPIO_PIN_1, PIN_21 },

       [PIN_GPIOA3_4] = { GPIOA3_BASE, GPIO_PIN_4, PIN_18 },
       [PIN_GPIOA3_5] = { GPIOA3_BASE, GPIO_PIN_5, PIN_20 },
       [PIN_GPIOA3_6] = { GPIOA3_BASE, GPIO_PIN_6, PIN_53 },
       [PIN_GPIOA3_7] = { GPIOA3_BASE, GPIO_PIN_7, PIN_45 },
};

static uint8_t GpioGetPinIndex(uint32_t base, uint8_t pinMask)
{
    /*TODO:*/
}

static void GpioEnableClocks(uint32_t base)
{
    switch (base)
    {
        case GPIOA0_BASE:
            PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
            break;

        case GPIOA1_BASE:
            PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
            break;

        case GPIOA2_BASE:
            PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);
            break;

        case GPIOA3_BASE:
            PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
            break;

        default:
            /* should never reach here */
            ASSERT(false);
            break;
    }
}

static uint32_t GpioMapStrength(PIN_STRENGTH strength)
{
    switch (strength)
    {
        case PIN_STRENGTH_LOW:
            return PIN_STRENGTH_2MA;

        case PIN_STRENGTH_MEDIUM:
            return PIN_STRENGTH_4MA;

        case PIN_STRENGTH_HIGH:
            return PIN_STRENGTH_6MA;

        default:
            ASSERT(false);
            return PIN_STRENGTH_2MA;
    }
}

static uint32_t GpioMapPinType(PIN_TYPES pull, PIN_CONFIGS cfg)
{
    if (cfg == PIN_CONFIG_OPEN_DRAIN)
    {
        switch (pull)
        {
            case PIN_TYPE_PULL_UP:
                return PIN_TYPE_OD_PU;

            case PIN_TYPE_PULL_DOWN:
                return PIN_TYPE_OD_PD;

            default:
                return PIN_TYPE_OD;
        }
    }
    else
    {
        switch (pull)
        {
            case PIN_TYPE_PULL_UP:
                return PIN_TYPE_STD_PU;

            case PIN_TYPE_PULL_DOWN:
                return PIN_TYPE_STD_PD;

            default:
                return PIN_TYPE_STD;
        }
    }
}

static uint32_t GpioMapEdge(PIN_IRQ_MODES mode)
{
    switch (mode)
    {
        case PIN_IRQ_RISING:
            return GPIO_RISING_EDGE;

        case PIN_IRQ_FALLING:
            return GPIO_FALLING_EDGE;

        case PIN_IRQ_BOTH:
            return GPIO_BOTH_EDGES;

        default:
            ASSERT(false);
            return GPIO_FALLING_EDGE;
    }
}

void GpioInit(GpioHandle_t* const handle,
              uint8_t pin,
              PIN_MODES mode,
              PIN_TYPES pull,
              PIN_STRENGTH strength,
              PIN_CONFIGS config,
              uint32_t value)
{
    ASSERT(handle != NULL);
    ASSERT(pin < PIN_GPIO_MAX);

    if (pin == PIN_NC)
    {
        return;
    }

    const GpioMap_t* map = &m_GPIO_MAP[pin];

    handle->hw.cc3220.base = map->base;
    handle->hw.cc3220.pinMask = map->mask;
    handle->irqHandler = NULL;

    GpioEnableClocks(map->base);

    uint32_t pinType = GpioMapPinType(pull, config);
    uint32_t pinStrength = GpioMapStrength(strength);

    PinConfigSet(map->pin, pinStrength, pinType);

    switch (mode)
    {
        case PIN_MODE_INPUT:
            PinModeSet(map->pin, PIN_MODE_0);
            GPIODirModeSet(map->base, map->mask, GPIO_DIR_MODE_IN);
            break;

        case PIN_MODE_OUTPUT:
        {
            PinModeSet(map->pin, PIN_MODE_0);
            GPIODirModeSet(map->base, map->mask, GPIO_DIR_MODE_OUT);

            uint8_t newValue = (value ? 1 : 0);
            GPIOPinWrite(map->base, map->mask, newValue);
        }
        break;

        case PIN_MODE_ALTERNATE:
        {
            uint32_t pinMode = value;
            PinModeSet(map->pin, pinMode);
        }
        break;

        case PIN_MODE_ANALOG:
            /* TODO: */
            break;

        default:
            ASSERT(false);
            break;
    }
}

void GpioWrite(const GpioHandle_t* const handle, PIN_STATES state)
{
    ASSERT(handle != NULL);

    uint32_t base  = handle->hw.cc3220.base;
    uint8_t pinMask = handle->hw.cc3220.pinMask;

    uint8_t value = (state == PIN_STATE_HIGH) ? pinMask : 0U;

    GPIOPinWrite(base, pinMask, value);
}

uint16_t GpioRead(const GpioHandle_t* const handle)
{
    ASSERT(handle != NULL);

    uint32_t base = handle->hw.cc3220.base;
    uint8_t pinMask = handle->hw.cc3220.pinMask;

    return (uint16_t)(GPIOPinRead(base, pinMask) ? 1U : 0U);
}

void GpioToggle(const GpioHandle_t* const handle)
{
    ASSERT(handle != NULL);

    uint32_t base = handle->hw.cc3220.base;
    uint8_t pinMask = handle->hw.cc3220.pinMask;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint32_t value = GPIOPinRead(base, pinMask);
    uint8_t newValue = (value ? 0U : pinMask);

    GPIOPinWrite(base, pinMask, newValue);

    __set_PRIMASK(primask);
}

void GpioSetInterrupt(GpioHandle_t* const handle, PIN_IRQ_MODES mode, uint8_t priority, GpioIrqHandler handler)
{
    ASSERT(handle != NULL);

    if (mode == PIN_IRQ_NONE)
    {
        return;
    }

    uint32_t base = handle->hw.cc3220.base;
    uint8_t pinMask = handle->hw.cc3220.pinMask;
    uint8_t pinIndex = GpioGetPinIndex(base, pinMask);

    GPIOIntDisable(base, pinMask);
    GPIOIntClear(base, pinMask);

    uint32_t edge = GpioMapEdge(mode);

    GPIOIntTypeSet(base, pinMask, edge);

    handle->irqHandler = handler;

    for (uint8_t i = 0; i < PIN_GPIO_MAX; i++)
    {
        if (m_GpioIrq[i] != NULL && m_GpioIrq[i]->hw.cc3220.pinIndex == pinIndex)
        {
            return;
        }
    }
}





