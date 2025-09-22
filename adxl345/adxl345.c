#include <stddef.h>
#include <string.h>

#include "adxl345.h"
#include "adxl345-regs.h"
#include "spi.h"

#if 1
#include "i2c.h"
#define ADXL345_I2C_ADDRESS 0x53
#define ADXL345_ID          0xE5
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

static AdxlRegisterRequest_t m_adxlRegisterRequest;
static AdxlVectorRequest_t m_adxlVectorRequest;
static Adxl345_t m_adxl345;

#if 1
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
        acceleration.y = (int16_t)(req->rx[4] << 8 | req->rx[3]);;
        acceleration.z = (int16_t)(req->rx[6] << 8 | req->rx[5]);;

        req->callback(&acceleration, req->userContext);
    }
}

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

bool ADXL_SelfTestOverI2C(void)
{
    uint8_t adxlIdOut = 0;
    uint8_t adxlThresTapIn = 0xAA;
    uint8_t adxlThresTapOut = 0;
    uint8_t adxlPowerControlIn = 0x08;
    uint8_t adxlPowerControlOut = 0;
    uint8_t adxlIdAddress = ADXL345_DEVID;
    uint8_t adxlThresTapAddress = ADXL345_THRESH_TAP;
    uint8_t adxlDataX0Address = ADXL345_DATAX0;
    uint8_t adxlPowerControlAddress = ADXL345_POWER_CTL;
    uint8_t adxlBuff[2] = { 0 };
    uint8_t adxlVector[6] = { 0 };

    Acceleration_t acceleration = { .x = 0, .y = 0, .z = 0 };

    I2C_Init(&m_i2c, I2C_1);

    /* read adxl345 ID */
    I2C_Transaction_t tranction;

    tranction.TxRxState = I2C_IDLE;
    tranction.devAddress = ADXL345_I2C_ADDRESS;
    tranction.txBuffer = &adxlIdAddress;
    tranction.txLen = sizeof(adxlIdAddress);
    tranction.rxBuffer = NULL;
    tranction.rxLen = 0;
    tranction.onTxDone = NULL;
    tranction.onRxDone = NULL;

    I2C_MasterTransmit_IT(&m_i2c, &tranction);

    DelayMs(1);

    tranction.TxRxState = I2C_IDLE;
    tranction.devAddress = ADXL345_I2C_ADDRESS;
    tranction.txBuffer = NULL;
    tranction.txLen = 0;
    tranction.rxBuffer = &adxlIdOut;
    tranction.rxLen = sizeof(adxlIdOut);
    tranction.onTxDone = NULL;
    tranction.onRxDone = NULL;

    I2C_MasterReceive_IT(&m_i2c, &tranction);

    DelayMs(1);

    if (adxlIdOut != ADXL345_ID)
    {
        return false;
    }

    /* write to adxl345 THRES_TAP reg value 0xAA */
    adxlBuff[0] = adxlThresTapAddress;
    adxlBuff[1] = adxlThresTapIn;

    tranction.TxRxState = I2C_IDLE;
    tranction.devAddress = ADXL345_I2C_ADDRESS;
    tranction.txBuffer = adxlBuff;
    tranction.txLen = sizeof(adxlBuff);
    tranction.rxBuffer = NULL;
    tranction.rxLen = 0;
    tranction.onTxDone = NULL;
    tranction.onRxDone = NULL;

    I2C_MasterTransmit_IT(&m_i2c, &tranction);

    DelayMs(1);

    tranction.TxRxState = I2C_IDLE;
    tranction.devAddress = ADXL345_I2C_ADDRESS;
    tranction.txBuffer = NULL;
    tranction.txLen = 0;
    tranction.rxBuffer = &adxlThresTapOut;
    tranction.rxLen = sizeof(adxlThresTapOut);
    tranction.onTxDone = NULL;
    tranction.onRxDone = NULL;

    I2C_MasterReceive_IT(&m_i2c, &tranction);

    DelayMs(1);

    if (adxlThresTapIn != adxlThresTapOut)
    {
        return false;
    }

    /* enable measurement */
    adxlBuff[0] = adxlPowerControlAddress;
    adxlBuff[1] = adxlPowerControlIn;

    tranction.TxRxState = I2C_IDLE;
    tranction.devAddress = ADXL345_I2C_ADDRESS;
    tranction.txBuffer = adxlBuff;
    tranction.txLen = sizeof(adxlBuff);
    tranction.rxBuffer = NULL;
    tranction.rxLen = 0;
    tranction.onTxDone = NULL;
    tranction.onRxDone = NULL;

    I2C_MasterTransmit_IT(&m_i2c, &tranction);

    DelayMs(1);

    tranction.TxRxState = I2C_IDLE;
    tranction.devAddress = ADXL345_I2C_ADDRESS;
    tranction.txBuffer = NULL;
    tranction.txLen = 0;
    tranction.rxBuffer = &adxlPowerControlOut;
    tranction.rxLen = sizeof(adxlPowerControlOut);
    tranction.onTxDone = NULL;
    tranction.onRxDone = NULL;

    I2C_MasterReceive_IT(&m_i2c, &tranction);

    DelayMs(1);

    if (adxlPowerControlIn != adxlPowerControlOut)
    {
        return false;
    }

    tranction.TxRxState = I2C_IDLE;
    tranction.devAddress = ADXL345_I2C_ADDRESS;
    tranction.txBuffer = &adxlDataX0Address;
    tranction.txLen = sizeof(adxlDataX0Address);
    tranction.rxBuffer = NULL;
    tranction.rxLen = 0;
    tranction.onTxDone = NULL;
    tranction.onRxDone = NULL;

    I2C_MasterTransmit_IT(&m_i2c, &tranction);

    DelayMs(1);

    tranction.TxRxState = I2C_IDLE;
    tranction.devAddress = ADXL345_I2C_ADDRESS;
    tranction.txBuffer = NULL;
    tranction.txLen = 0;
    tranction.rxBuffer = adxlVector;
    tranction.rxLen = sizeof(adxlVector);
    tranction.onTxDone = NULL;
    tranction.onRxDone = NULL;

    I2C_MasterReceive_IT(&m_i2c, &tranction);

    DelayMs(1);

    acceleration.x = (int16_t)(adxlVector[1] << 8 | adxlVector[0]);
    acceleration.y = (int16_t)(adxlVector[3] << 8 | adxlVector[2]);
    acceleration.z = (int16_t)(adxlVector[5] << 8 | adxlVector[4]);

    if (acceleration.x == 0 && acceleration.y == 0 && acceleration.z == 0)
    {
        return false;
    }

    return true;
}
