#include <stddef.h>
#include <string.h>

#include "assert.h"
#include "adxl345.h"
#include "adxl345-regs.h"
#include "spi.h"

#include "i2c.h"
#define ADXL345_I2C_ADDRESS     0x53

#define SPI_CLOCK_RATE  5000000     /* Hz */

/*TODO:*/
#if 0
AdxlInterface_t g_adxlSpiDriver = {
        .init = &ADXL_Init,
        .readRegisterAsync = &ADXL_ReadRegisterAsync,
        .writeRegisterAsync = &ADXL_WriteRegisterAsync,
};

AdxlInterface_t g_adxlI2CDriver = {
        .init = &ADXL_InitI2C,
        .readRegisterAsync = &ADXL_ReadRegisterAsyncI2C,
        .writeRegisterAsync = &ADXL_WriteRegisterAsyncI2C,
};
#endif

typedef struct
{
    Spi_t spi;
    Gpio_t nss;
} Adxl345_t;

typedef enum
{
    BUS_SPI = 0,
    BUS_I2C,
} BUS_TYPE;

typedef struct
{
    uint8_t tx[7];
    uint8_t rx[7];
    uint8_t length;
    BUS_TYPE bus;
    ADXL_RequestHandler_t callback;
    void* userContext;
} AdxlRequest_t;

static AdxlRequest_t m_adxlRequest;
static Adxl345_t m_adxl345;
static I2C_Handle_t m_i2c;

static void AdxlCreateRequest(  AdxlRequest_t* req,
                                BUS_TYPE bus,
                                uint8_t* tx, uint8_t txLen,
                                uint8_t* rx, uint8_t rxLen,
                                ADXL_RequestHandler_t callback,
                                void* context);

static void Adxl345_Activate(void* context)
{
    GpioWrite(&m_adxl345.nss, 0);
}

static void Adxl345_Deactivate(void* context)
{
    GpioWrite(&m_adxl345.nss, 1);
}

static void OnRegisterRequestCompleted(void* context)
{
    Acceleration_t acceleration;
    AdxlRequest_t* request = (AdxlRequest_t*)context;

    switch (request->bus)
    {
        case BUS_SPI:
            if (request->length == 0)
            {
                if (request->callback)
                {
                    request->callback(NULL, request->userContext);
                }
            }

            if (request->length == 2)
            {
                if (request->callback)
                {
                    request->callback(&request->rx[1], request->userContext);
                }
            }

            if (request->length == 7)
            {
                if (request->callback)
                {
                    acceleration.x = (int16_t)(request->rx[2] << 8 | request->rx[1]);
                    acceleration.y = (int16_t)(request->rx[4] << 8 | request->rx[3]);
                    acceleration.z = (int16_t)(request->rx[6] << 8 | request->rx[5]);

                    request->callback(&acceleration, request->userContext);
                }
            }
            break;

        case BUS_I2C:
            if (request->length == 1)
            {
                if (request->callback)
                {
                    request->callback(&request->rx[0], request->userContext);
                }
            }

            if (request->length == 6)
            {
                if (request->callback)
                {
                    acceleration.x = (int16_t)(request->rx[1] << 8 | request->rx[0]);
                    acceleration.y = (int16_t)(request->rx[3] << 8 | request->rx[2]);
                    acceleration.z = (int16_t)(request->rx[5] << 8 | request->rx[4]);

                    request->callback(&acceleration, request->userContext);
                }
            }
            break;
        default:
            ASSERT(false);
            break;
    }
}

static void I2C_OnRegisterWriteCompleted(void* context)
{
    AdxlRequest_t* request = (AdxlRequest_t*)context;

    if (request->length == 0)
    {
        if (request->callback != NULL)
        {
            (*request->callback)(NULL, request->userContext);
        }
    }
    else
    {
        I2C_Transaction_t i2cTransaction = {
            .devAddress = ADXL345_I2C_ADDRESS,
            .txBuffer = NULL,
            .txLen = 0,
            .rxBuffer = request->rx,
            .rxLen = request->length,
            .onTxDone = NULL,
            .onRxDone = &OnRegisterRequestCompleted,
            .context = request,
        };

        I2C_MasterReceive_IT(&m_i2c, &i2cTransaction);
    }
}

void ADXL_InitSPI(void)
{
    SpiInit(&m_adxl345.spi, SPI_1, CPOL_1, CPHA_1, SPI_CLOCK_RATE);

    /* chip select gpio */
    GpioInit(&m_adxl345.nss, PA_0, PIN_MODE_OUTPUT, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, 1);
}

void ADXL_ReadRegisterAsyncSPI(uint8_t address, ADXL_RequestHandler_t callback, void* context)
{
    uint8_t tx[2] = {0x80 | address, 0xFF};

    AdxlCreateRequest(&m_adxlRequest, BUS_SPI, tx, sizeof(tx), m_adxlRequest.rx, 2, callback, context);

    SpiTransaction_t spiTransaction = {
        .txBuffer = m_adxlRequest.tx,
        .rxBuffer = m_adxlRequest.rx,
        .txLen = sizeof(tx),
        .rxLen = m_adxlRequest.length,
        .preTransaction = &Adxl345_Activate,
        .postTransaction = &Adxl345_Deactivate,
        .onTransactionDone = &OnRegisterRequestCompleted,
        .context = &m_adxlRequest
    };

    SpiTransfer_IT(&m_adxl345.spi, &spiTransaction);
}

void ADXL_WriteRegisterAsyncSPI(uint8_t address, ADXL_RequestHandler_t callback, void* value)
{
    uint8_t tx[2];
    tx[0] = 0x00 | address;
    tx[1] = *(uint8_t*)value;

    AdxlCreateRequest(&m_adxlRequest, BUS_SPI, tx, sizeof(tx), m_adxlRequest.rx, 0, callback, NULL);

    SpiTransaction_t spiTransaction = {
        .txBuffer = m_adxlRequest.tx,
        .rxBuffer = NULL,
        .txLen = sizeof(tx),
        .rxLen = m_adxlRequest.length,
        .preTransaction = &Adxl345_Activate,
        .postTransaction = &Adxl345_Deactivate,
        .onTransactionDone = &OnRegisterRequestCompleted,
        .context = &m_adxlRequest
    };

    SpiTransfer_IT(&m_adxl345.spi, &spiTransaction);
}

void ADXL_ReadVectorAsyncSPI(uint8_t address, ADXL_RequestHandler_t callback, void* context)
{
    uint8_t tx[7];
    memset(tx, 0xff, sizeof(tx));
    tx[0] = 0x80 | 0x40 | address;

    AdxlCreateRequest(&m_adxlRequest, BUS_SPI, tx, sizeof(tx), m_adxlRequest.rx, 7, callback, context);

    SpiTransaction_t spiTransaction = {
        .txBuffer = m_adxlRequest.tx,
        .rxBuffer = m_adxlRequest.rx,
        .txLen = sizeof(tx),
        .rxLen = m_adxlRequest.length,
        .preTransaction = &Adxl345_Activate,
        .postTransaction = &Adxl345_Deactivate,
        .onTransactionDone = OnRegisterRequestCompleted,
        .context = &m_adxlRequest
    };

    SpiTransfer_IT(&m_adxl345.spi, &spiTransaction);
}

void ADXL_InitI2C(void)
{
#if 0
    m_i2c.config.speed = I2C_SPEED_FAST_MODE;
    m_i2c.config.dutyCycle = 1;
    m_i2c.config.ackControl = 1;
#else
    m_i2c.config.speed = I2C_SPEED_STANDARD_MODE;
    m_i2c.config.dutyCycle = 0;
    m_i2c.config.ackControl = 1;
#endif
    I2C_Init(&m_i2c, I2C_1);
}

void ADXL_ReadRegisterAsyncI2C(uint8_t address, ADXL_RequestHandler_t callback, void* context)
{
    uint8_t tx[1] = { address };

    AdxlCreateRequest(&m_adxlRequest, BUS_I2C, tx, sizeof(tx), m_adxlRequest.rx, 1, callback, context);

    I2C_Transaction_t i2cTransaction = {
        .devAddress = ADXL345_I2C_ADDRESS,
        .txBuffer = &m_adxlRequest.tx[0],
        .txLen = sizeof(tx),
        .rxBuffer = m_adxlRequest.rx,
        .rxLen = m_adxlRequest.length,
        .onTxDone = &I2C_OnRegisterWriteCompleted,
        .onRxDone = NULL,
        .context = &m_adxlRequest
    };

    I2C_MasterTransmit_IT(&m_i2c, &i2cTransaction);
}

void ADXL_WriteRegisterAsyncI2C(uint8_t address, ADXL_RequestHandler_t callback, void* value)
{
    uint8_t tx[2];
    tx[0] = address;
    tx[1] = *(uint8_t*)value;

    AdxlCreateRequest(&m_adxlRequest, BUS_I2C, tx, sizeof(tx), m_adxlRequest.rx, 0, callback, NULL);

    I2C_Transaction_t i2cTransaction = {
        .devAddress = ADXL345_I2C_ADDRESS,
        .txBuffer = m_adxlRequest.tx,
        .txLen = sizeof(tx),
        .rxBuffer = NULL,
        .rxLen = m_adxlRequest.length,
        .onTxDone = &I2C_OnRegisterWriteCompleted,
        .onRxDone = NULL,
        .context = &m_adxlRequest
    };

    I2C_MasterTransmit_IT(&m_i2c, &i2cTransaction);
}

void ADXL_ReadVectorAsyncI2C(uint8_t address, ADXL_RequestHandler_t callback, void* context)
{
    uint8_t tx[1] = { address };

    AdxlCreateRequest(&m_adxlRequest, BUS_I2C, tx, sizeof(tx), m_adxlRequest.rx, 6, callback, context);

    I2C_Transaction_t i2cTransaction = {
        .devAddress = ADXL345_I2C_ADDRESS,
        .txBuffer = m_adxlRequest.tx,
        .txLen = sizeof(uint8_t),
        .rxBuffer = m_adxlRequest.rx,
        .rxLen = m_adxlRequest.length,
        .onTxDone = &I2C_OnRegisterWriteCompleted,
        .onRxDone = NULL,
        .context = &m_adxlRequest
    };

    I2C_MasterTransmit_IT(&m_i2c, &i2cTransaction);
}

static void AdxlCreateRequest(  AdxlRequest_t* req,
                                BUS_TYPE bus,
                                uint8_t* tx, uint8_t txLen,
                                uint8_t* rx, uint8_t rxLen,
                                ADXL_RequestHandler_t callback,
                                void* context)
{
    ASSERT(req != NULL);
    ASSERT(tx != NULL);
    ASSERT(rx != NULL);
    ASSERT(txLen <= sizeof(req->tx));

    memset(req->tx, 0, sizeof(req->tx));
    memset(req->rx, 0, sizeof(req->rx));

    if (tx != NULL && txLen > 0)
    {
        memcpy(req->tx, tx, txLen);
    }

    req->length = rxLen;

    req->bus = bus;

    req->callback = callback;
    req->userContext = context;
}

/* TODO: */
#if 0
void AdxlInit(Adxl_t* dev, AdxlInterface_t* bus)
{
    dev->bus = bus;

    dev->bus->init();
}

void AdxlReadRegisterAsync(const Adxl_t* const dev, uint8_t address, ADXL_RequestHandler_t callback, void* context)
{
    ASSERT(dev != NULL);

    dev->bus->readRegisterAsync(address, callback, context);
}

void AdxlWriteRegisterAsync(const Adxl_t* const dev, uint8_t address, void* value)
{
    ASSERT(dev != NULL);

    dev->bus->writeRegisterAsync(address, value);
}
#endif
