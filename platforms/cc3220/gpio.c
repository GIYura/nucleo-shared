#include <stddef.h>

#include "custom-assert.h"
#include "gpio.h"

#include "gpio.h"

#define INTERRUPT_MAX    64

#include <ti/devices/cc32xx/driverlib/gpio.h>
#include <ti/devices/cc32xx/driverlib/pin.h>
#include <ti/devices/cc32xx/driverlib/prcm.h>
#include <ti/devices/cc32xx/driverlib/rom_map.h>
#include <ti/devices/cc32xx/inc/hw_ints.h>
#include <ti/devices/cc32xx/inc/hw_memmap.h>

/* ============================================================
 * INTERNAL IRQ TABLE — one callback per pin (0..63)
 * CC3220 реально использует 0..31 GPIO pins
 * ============================================================ */
static GpioHandle_t* m_GpioIrq[INTERRUPT_MAX] = {0};

/* ============================================================
 * INTERNAL: CC3220 uses one IRQ handler for all GPIOA ports
 * ============================================================ */
static void gpio_cc3220_global_isr(void)
{
    uint32_t status;

    /* PORT A0 */
    status = MAP_GPIOIntStatus(GPIOA0_BASE, true);
    if (status) {
        for (uint8_t pin = 0; pin < 8; pin++) {
            if (status & (1 << pin)) {
                MAP_GPIOIntClear(GPIOA0_BASE, 1 << pin);
                if (m_GpioIrq[pin] && m_GpioIrq[pin]->irqHandler)
                    m_GpioIrq[pin]->irqHandler();
            }
        }
    }

    /* PORT A1 */
    status = MAP_GPIOIntStatus(GPIOA1_BASE, true);
    if (status) {
        for (uint8_t pin = 0; pin < 8; pin++) {
            if (status & (1 << pin)) {
                MAP_GPIOIntClear(GPIOA1_BASE, 1 << pin);
                if (m_GpioIrq[8 + pin] && m_GpioIrq[8 + pin]->irqHandler)
                    m_GpioIrq[8 + pin]->irqHandler();
            }
        }
    }

    /* PORT A2 */
    status = MAP_GPIOIntStatus(GPIOA2_BASE, true);
    if (status) {
        for (uint8_t pin = 0; pin < 8; pin++) {
            if (status & (1 << pin)) {
                MAP_GPIOIntClear(GPIOA2_BASE, 1 << pin);
                if (m_GpioIrq[16 + pin] && m_GpioIrq[16 + pin]->irqHandler)
                    m_GpioIrq[16 + pin]->irqHandler();
            }
        }
    }

    /* PORT A3 */
    status = MAP_GPIOIntStatus(GPIOA3_BASE, true);
    if (status) {
        for (uint8_t pin = 0; pin < 8; pin++) {
            if (status & (1 << pin)) {
                MAP_GPIOIntClear(GPIOA3_BASE, 1 << pin);
                if (m_GpioIrq[24 + pin] && m_GpioIrq[24 + pin]->irqHandler)
                    m_GpioIrq[24 + pin]->irqHandler();
            }
        }
    }
}

/* ============================================================
 * INTERNAL: Enable all GPIO ports (A0..A3)
 * ============================================================ */
static void GpioEnableClocks(uint32_t pinNumber)
{
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
}

/* ============================================================
 * INTERNAL: Translate CC3220 pin number → base address + local pin
 * pinNumber = 0..31
 * ============================================================ */
static uint32_t gpio_cc3220_port_base(uint32_t pinNumber)
{
    if (pinNumber < 8) return GPIOA0_BASE;
    if (pinNumber < 16) return GPIOA1_BASE;
    if (pinNumber < 24) return GPIOA2_BASE;
    return GPIOA3_BASE;
}

static uint8_t gpio_cc3220_local_pin(uint32_t pinNumber)
{
    return (pinNumber % 8);
}

/* ============================================================
 * PUBLIC: GpioInit()
 * ============================================================ */
void GpioInit(GpioHandle_t* const handle,
              PIN_NAMES pinName,
              PIN_MODES mode,
              PIN_TYPES pull,
              PIN_STRENGTH strength,
              PIN_CONFIGS config,
              PIN_STATES initVal)
{
    ASSERT(handle != NULL);

    if (pinName == PIN_NC)
    {
        return;
    }

    handle->pin = pinName;

    /* Store hardware pin number */
    handle->hw.cc3220.pinNumber = pinName;

    GpioEnableClocks();

    uint32_t pin = pinName;

    /* ============================
     * Configure PINMUX + pull + strength
     * ============================ */
    uint32_t drv =
        (strength == PIN_STRENGTH_LOW)    ? PIN_STRENGTH_2MA :
        (strength == PIN_STRENGTH_MEDIUM) ? PIN_STRENGTH_4MA :
                                            PIN_STRENGTH_6MA;

    uint32_t pull_mode =
        (pull == PIN_TYPE_PULL_UP)   ? PIN_TYPE_STD_PU :
        (pull == PIN_TYPE_PULL_DOWN) ? PIN_TYPE_STD_PD :
                                       PIN_TYPE_STD;

    /* Apply driver and pull configuration */
    MAP_PinConfigSet(pin, drv, pull_mode);

    /* ============================
     * Direction / Mode
     * ============================ */
    if (mode == PIN_MODE_INPUT)
    {
        MAP_GPIOPinTypeGPIOInput(pin);
    }
    else if (mode == PIN_MODE_OUTPUT)
    {
        if (config == PIN_CONFIG_OPEN_DRAIN)
            MAP_GPIOPinTypeGPIOOutputOD(pin);
        else
            MAP_GPIOPinTypeGPIOOutput(pin);

        /* Initial output value */
        MAP_GPIOPinWrite(pin, initVal ? pin : 0);
    }
    else if (mode == PIN_MODE_ALTERNATE)
    {
        /* user must configure mux via PinModeSet() externally */
        MAP_PinTypeGPIO(pin, pull_mode);
    }
    else if (mode == PIN_MODE_ANALOG)
    {
        /* CC3220 does not really support analog mode in GPIO sense */
        MAP_PinTypeGPIO(pin, pull_mode);
    }
}

/* ============================================================
 * PUBLIC: GpioWrite()
 * ============================================================ */
void GpioWrite(const GpioHandle_t* const handle, PIN_STATES state)
{
    if (!handle) return;

    uint32_t pin = handle->hw.cc3220.pinNumber;

    MAP_GPIOPinWrite(pin, state ? pin : 0);
}

/* ============================================================
 * PUBLIC: GpioRead()
 * ============================================================ */
uint16_t GpioRead(const GpioHandle_t* const handle)
{
    if (!handle) return 0;

    uint32_t pin = handle->hw.cc3220.pinNumber;
    return MAP_GPIOPinRead(pin) ? 1 : 0;
}

/* ============================================================
 * PUBLIC: GpioToggle()
 * ============================================================ */
void GpioToggle(const GpioHandle_t* const handle)
{
    if (!handle) return;

    uint32_t pin = handle->hw.cc3220.pinNumber;

    uint32_t val = MAP_GPIOPinRead(pin);
    MAP_GPIOPinWrite(pin, val ? 0 : pin);
}

/* ============================================================
 * PUBLIC: GpioSetInterrupt()
 * ============================================================ */
void GpioSetInterrupt(GpioHandle_t* const handle,
                      PIN_IRQ_MODES mode,
                      uint8_t priority,
                      GpioIrqHandler handler)
{
    if (!handle || !handler)
        return;

    uint32_t pin = handle->hw.cc3220.pinNumber;
    uint32_t port = gpio_cc3220_port_base(pin);
    uint8_t  local = gpio_cc3220_local_pin(pin);

    handle->irqHandler = handler;

    /* Register global ISR only once */
    static uint8_t isr_registered = 0;
    if (!isr_registered)
    {
        MAP_GPIOIntRegister(port, gpio_cc3220_global_isr);
        isr_registered = 1;
    }

    /* Clear previous configuration */
    MAP_GPIOIntDisable(port, 1 << local);
    MAP_GPIOIntClear(port, 1 << local);

    /* Configure interrupt type */
    if (mode == PIN_IRQ_RISING)
        MAP_GPIOIntTypeSet(port, 1 << local, GPIO_RISING_EDGE);
    else if (mode == PIN_IRQ_FALLING)
        MAP_GPIOIntTypeSet(port, 1 << local, GPIO_FALLING_EDGE);
    else
        MAP_GPIOIntTypeSet(port, 1 << local, GPIO_BOTH_EDGES);

    /* Save handler in table */
    gpio_irq_table[pin] = handle;

    /* Enable interrupt in peripheral */
    MAP_GPIOIntEnable(port, 1 << local);

    /* Enable IRQ in NVIC */
    MAP_IntPrioritySet(INT_GPIOA0 + (pin / 8), priority << 5);
    MAP_IntEnable(INT_GPIOA0 + (pin / 8));
}

