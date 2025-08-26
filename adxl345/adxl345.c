#include <stddef.h>
#include <string.h>

#include "adxl345.h"
#include "adxl345-regs.h"
#include "spi.h"

#define SPI_CLOCK_RATE  5000000     /* Hz */

static Spi_t m_spi;
static Gpio_t m_nss;

typedef struct
{
    uint8_t tx[2];
    uint8_t rx[2];
    ADXL_RequestHandler_t callback;
    void* userContext;
} AdxlRequest_t;

static AdxlRequest_t m_adxlRequest;

static void SpiAppEventHandler(void* context)
{
    if (context == NULL)
    {
        GpioWrite(&m_nss, 1);
        return;
    }

    AdxlRequest_t* req = (AdxlRequest_t*)context;

    GpioWrite(&m_nss, 1);

    if (req->callback)
    {
        req->callback(req->rx[1], req->userContext);
    }
}

void ADXL_Init(void)
{
    SpiInit(&m_spi, SPI_1, CPOL_1, CPHA_1, SPI_CLOCK_RATE);

    /* chip select gpio */
    GpioInit(&m_nss, PA_0, PIN_MODE_OUTPUT, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, 1);

    memset(&m_adxlRequest.tx, 0, sizeof(m_adxlRequest.tx));
    memset(&m_adxlRequest.rx, 0, sizeof(m_adxlRequest.rx));
    m_adxlRequest.callback = NULL;
    m_adxlRequest.userContext = NULL;

    SpiRegisterRxHandler(&m_spi, &SpiAppEventHandler);
    SpiRegisterTxHandler(&m_spi, &SpiAppEventHandler);
}

void ADXL_ReadRegisterAsync(uint8_t address, ADXL_RequestHandler_t callback, void* context)
{
    m_adxlRequest.callback = callback;
    m_adxlRequest.userContext = context;

    m_adxlRequest.tx[0] = 0x80 | address;
    m_adxlRequest.tx[1] = 0xFF;
    m_adxlRequest.rx[0] = 0;
    m_adxlRequest.rx[1] = 0;

    GpioWrite(&m_nss, 0);

    SpiTransfer_IT(&m_spi, m_adxlRequest.tx, m_adxlRequest.rx, sizeof(m_adxlRequest.rx), &m_adxlRequest);
}

void ADXL_WriteRegisterAsync(uint8_t address, void* value)
{
    uint8_t* regValue = (uint8_t*)value;

    m_adxlRequest.tx[0] = 0x00 | address;
    m_adxlRequest.tx[1] = *regValue;

    GpioWrite(&m_nss, 0);

    SpiTransfer_IT(&m_spi, m_adxlRequest.tx, NULL, sizeof(m_adxlRequest.tx), NULL);
}
