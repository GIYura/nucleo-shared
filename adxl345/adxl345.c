#include <stddef.h>
#include <string.h>

#include "adxl345.h"
#include "adxl345-regs.h"
#include "spi.h"

#if 1
#include "i2c.h"
#define ADXL345_I2C_ADDRESS     0x53
#define ADXL345_VECTOR_SIZE     (6) /* bytes */
#define ADXL345_REGISTER_SIZE   (1) /* bytes */
#endif

#define SPI_CLOCK_RATE  5000000     /* Hz */

typedef struct
{
    Spi_t spi;
    Gpio_t nss;
} Adxl345_t;

typedef struct
{
    uint8_t tx[2];
    uint8_t rx[2];
    ADXL_RequestHandler_t callback;
    void* userContext;
} AdxlRegisterRequest_t;

typedef struct
{
    uint8_t tx[7];
    uint8_t rx[7];
    ADXL_RequestHandler_t callback;
    void* userContext;
} AdxlVectorRequest_t;

/*
 * TODO:
 * This struct should be used as common for SPI/I2C transactions
 * (for single reg read/write and for reg dump)
 * */
typedef struct
{
    uint8_t tx[7];
    uint8_t rx[7];
    uint8_t rxLen;
    ADXL_RequestHandler_t callback;
    void* userContext;
} AdxlRequest_t;

static AdxlRegisterRequest_t m_adxlRegisterRequest;
static AdxlVectorRequest_t m_adxlVectorRequest;
static Adxl345_t m_adxl345;

#if 1
static AdxlRequest_t m_adxlRequest;
static I2C_Handle_t m_i2c;
#endif

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
    if (context == NULL)
    {
        return;
    }

    AdxlRegisterRequest_t* req = (AdxlRegisterRequest_t*)context;

    if (req->callback)
    {
        req->callback(&req->rx[1], req->userContext);
    }
}

static void I2C_OnRegisterRequestCompleted(void* context)
{
    AdxlRequest_t* req = (AdxlRequest_t*)context;

    if (req->rxLen == ADXL345_REGISTER_SIZE)
    {
        if (req->callback)
        {
            req->callback(&req->rx[0], req->userContext);
        }
    }

    if (req->rxLen == ADXL345_VECTOR_SIZE)
    {
        if (req->callback)
        {
            Acceleration_t acceleration;

            acceleration.x = (int16_t)(req->rx[1] << 8 | req->rx[0]);
            acceleration.y = (int16_t)(req->rx[3] << 8 | req->rx[2]);
            acceleration.z = (int16_t)(req->rx[5] << 8 | req->rx[4]);

            req->callback(&acceleration, req->userContext);
        }
    }
}

static void I2C_OnRegisterWriteCompleted(void* context)
{
    AdxlRequest_t* req = (AdxlRequest_t*)context;

    if (req->rxLen == 0)
    {
        if (req->callback != NULL)
        {
            req->callback(&req->rx[0], req->userContext);
        }
    }
    else
    {
        I2C_Transaction_t i2cTransaction = {
            .devAddress = ADXL345_I2C_ADDRESS,
            .txBuffer = NULL,
            .txLen = 0,
            .rxBuffer = req->rx,
            .rxLen = req->rxLen,
            .onTxDone = NULL,
            .onRxDone = &I2C_OnRegisterRequestCompleted,
            .context = req,
        };

        I2C_MasterReceive_IT(&m_i2c, &i2cTransaction);
    }
}

static void OnVectorRequestCompleted(void* context)
{
    if (context == NULL)
    {
        return;
    }

    AdxlVectorRequest_t* req = (AdxlVectorRequest_t*)context;

    if (req->callback)
    {
        Acceleration_t acceleration;

        acceleration.x = (int16_t)(req->rx[2] << 8 | req->rx[1]);
        acceleration.y = (int16_t)(req->rx[4] << 8 | req->rx[3]);
        acceleration.z = (int16_t)(req->rx[6] << 8 | req->rx[5]);

        req->callback(&acceleration, req->userContext);
    }
}

/*
 * SPI section
 * */
void ADXL_Init(void)
{
    SpiInit(&m_adxl345.spi, SPI_1, CPOL_1, CPHA_1, SPI_CLOCK_RATE);

    /* chip select gpio */
    GpioInit(&m_adxl345.nss, PA_0, PIN_MODE_OUTPUT, PIN_TYPE_NO_PULL_UP_PULL_DOWN, PIN_SPEED_FAST, PIN_CONFIG_PUSH_PULL, 1);

    memset(&m_adxlRegisterRequest.tx, 0, sizeof(m_adxlRegisterRequest.tx));
    memset(&m_adxlRegisterRequest.rx, 0, sizeof(m_adxlRegisterRequest.rx));
    m_adxlRegisterRequest.callback = NULL;
    m_adxlRegisterRequest.userContext = NULL;

    memset(&m_adxlVectorRequest.tx, 0, sizeof(m_adxlVectorRequest.tx));
    memset(&m_adxlVectorRequest.rx, 0, sizeof(m_adxlVectorRequest.rx));
    m_adxlVectorRequest.callback = NULL;
    m_adxlVectorRequest.userContext = NULL;
}

void ADXL_ReadRegisterAsync(uint8_t address, ADXL_RequestHandler_t callback, void* context)
{
    m_adxlRegisterRequest.callback = callback;
    m_adxlRegisterRequest.userContext = context;

    memset(&m_adxlRegisterRequest.tx, 0xff, sizeof(m_adxlRegisterRequest.tx));
    memset(&m_adxlRegisterRequest.rx, 0, sizeof(m_adxlRegisterRequest.rx));

    m_adxlRegisterRequest.tx[0] = 0x80 | address;

    SpiTransaction_t spiTransaction = {
        .txBuffer = m_adxlRegisterRequest.tx,
        .rxBuffer = m_adxlRegisterRequest.rx,
        .txLen = sizeof(m_adxlRegisterRequest.tx),
        .rxLen = sizeof(m_adxlRegisterRequest.rx),
        .preTransaction = &Adxl345_Activate,
        .postTransaction = &Adxl345_Deactivate,
        .onTransactionDone = &OnRegisterRequestCompleted,
        .context = &m_adxlRegisterRequest
    };

    SpiTransfer_IT(&m_adxl345.spi, &spiTransaction);
}



void ADXL_WriteRegisterAsync(uint8_t address, void* value)
{
    m_adxlRegisterRequest.tx[0] = 0x00 | address;
    m_adxlRegisterRequest.tx[1] = *(uint8_t*)value;

    SpiTransaction_t spiTransaction = {
        .txBuffer = m_adxlRegisterRequest.tx,
        .rxBuffer = NULL,
        .txLen = sizeof(m_adxlRegisterRequest.tx),
        .rxLen = 0,
        .preTransaction = &Adxl345_Activate,
        .postTransaction = &Adxl345_Deactivate,
        .onTransactionDone = &OnRegisterRequestCompleted,
        .context = NULL
    };

    SpiTransfer_IT(&m_adxl345.spi, &spiTransaction);
}

void ADXL_ReadVectorAsync(uint8_t address, ADXL_RequestHandler_t callback, void* context)
{
    m_adxlVectorRequest.callback = callback;
    m_adxlVectorRequest.userContext = context;

    memset(&m_adxlVectorRequest.tx, 0xff, sizeof(m_adxlVectorRequest.tx));
    memset(&m_adxlVectorRequest.rx, 0, sizeof(m_adxlVectorRequest.rx));

    m_adxlVectorRequest.tx[0] = 0x80 | 0x40 | address;

    SpiTransaction_t spiTransaction = {
        .txBuffer = m_adxlVectorRequest.tx,
        .rxBuffer = m_adxlVectorRequest.rx,
        .txLen = sizeof(m_adxlVectorRequest.tx),
        .rxLen = sizeof(m_adxlVectorRequest.rx),
        .preTransaction = &Adxl345_Activate,
        .postTransaction = &Adxl345_Deactivate,
        .onTransactionDone = &OnVectorRequestCompleted,
        .context = &m_adxlVectorRequest
    };

    SpiTransfer_IT(&m_adxl345.spi, &spiTransaction);
}

/*
 * I2C section
 * */
void ADXL_InitI2C(void)
{
    I2C_Init(&m_i2c, I2C_1);
}

void ADXL_ReadRegisterAsyncI2C(uint8_t address, ADXL_RequestHandler_t callback, void* context)
{
    m_adxlRequest.callback = callback;
    m_adxlRequest.userContext = context;

    m_adxlRequest.tx[0] = address;
    m_adxlRequest.rxLen = ADXL345_REGISTER_SIZE;

    I2C_Transaction_t i2cTransaction = {
        .devAddress = ADXL345_I2C_ADDRESS,
        .txBuffer = &m_adxlRequest.tx[0],
        .txLen = ADXL345_REGISTER_SIZE,
        .rxBuffer = NULL,
        .rxLen = 0,
        .onTxDone = &I2C_OnRegisterWriteCompleted,
        .onRxDone = NULL,
        .context = &m_adxlRequest,
    };

    I2C_MasterTransmit_IT(&m_i2c, &i2cTransaction);
}

void ADXL_WriteRegisterAsyncI2C(uint8_t address, ADXL_RequestHandler_t callback, void* value)
{
    uint8_t size = 0;

    m_adxlRequest.callback = callback;
    m_adxlRequest.userContext = NULL;

    m_adxlRequest.tx[size++] = address;
    m_adxlRequest.tx[size++] = *(uint8_t*)value;
    m_adxlRequest.rxLen = 0;

    I2C_Transaction_t i2cTransaction = {
        .devAddress = ADXL345_I2C_ADDRESS,
        .txBuffer = m_adxlRequest.tx,
        .txLen = size,
        .rxBuffer = NULL,
        .rxLen = 0,
        .onTxDone = &I2C_OnRegisterWriteCompleted,
        .onRxDone = NULL,
        .context = &m_adxlRequest,
    };

    I2C_MasterTransmit_IT(&m_i2c, &i2cTransaction);
}

void ADXL_ReadVectorAsyncI2C(uint8_t address, ADXL_RequestHandler_t callback, void* context)
{
    m_adxlRequest.callback = callback;
    m_adxlRequest.userContext = context;

    m_adxlRequest.tx[0] = address;
    m_adxlRequest.rxLen = ADXL345_VECTOR_SIZE;

    I2C_Transaction_t i2cTransaction = {
        .devAddress = ADXL345_I2C_ADDRESS,
        .txBuffer = &m_adxlRequest.tx[0],
        .txLen = sizeof(uint8_t),
        .rxBuffer = NULL,
        .rxLen = 0,
        .onTxDone = &I2C_OnRegisterWriteCompleted,
        .onRxDone = NULL,
        .context = &m_adxlRequest,
    };

    I2C_MasterTransmit_IT(&m_i2c, &i2cTransaction);
}

