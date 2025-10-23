//#include <Arduino.h>

#include <assert.h>
#include <string.h>


#if 0
#include "app-lora-config.h"
#include "radio.h"
#include "board.h"
//#include "timer.h"
#include "sx126x-regs.h"

/* SX126x intsanse */
static SX126x m_sx1262;

static void RadioInit(SX1262_ID id, RadioEvents_t* events, const GpioConfig_t* const config);
static void RadioReInit(SX1262_ID id, RadioEvents_t* events);
static RadioState_t RadioGetStatus(SX1262_ID id);
static void RadioSetModem(SX1262_ID id, RadioModems_t modem);
static void RadioSetChannel(SX1262_ID id, uint32_t freq);
static bool RadioIsChannelFree(SX1262_ID id, RadioModems_t modem, uint32_t freq, int16_t rssiThresh, uint32_t maxCarrierSenseTime);
static uint32_t RadioRandom(SX1262_ID id);
static void RadioSetRxConfig(   SX1262_ID id,
                                RadioModems_t modem,
                                uint32_t bandwidthIndex,
                                uint32_t datarate,
                                uint8_t coderate,
                                uint32_t bandwidthAfc,
                                uint16_t preambleLen,
                                uint16_t symbTimeout,
                                bool fixLen,
                                uint8_t payloadLen,
                                bool crcOn,
                                bool FreqHopOn,
                                uint8_t HopPeriod,
                                bool irqInverted,
                                bool rxContinuous);

static void RadioSetTxConfig(   SX1262_ID id,
                                RadioModems_t modem,
                                int8_t power,
                                uint32_t fdev,
                                uint32_t bandwidth, 
                                uint32_t datarate,
                                uint8_t coderate,
                                uint16_t preambleLen,
                                bool fixLen,
                                bool crcOn,
                                bool FreqHopOn,
                                uint8_t HopPeriod,
                                bool iqInverted,
                                uint32_t timeout);

static bool RadioCheckRfFrequency(SX1262_ID id, uint32_t frequency);
static uint32_t RadioTimeOnAir(SX1262_ID id, RadioModems_t modem, uint8_t pktLen);
static void RadioSend(SX1262_ID id, const uint8_t* const buffer, uint8_t size);
static void RadioSleep(SX1262_ID id);
static void RadioStandby(SX1262_ID id);
static void RadioRx(SX1262_ID id, uint32_t timeout);
static void RadioSetCadParams(SX1262_ID id, uint8_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin, uint8_t cadExitMode, uint32_t cadTimeout);
static void RadioStartCad(SX1262_ID id);
static void RadioSetTxContinuousWave(SX1262_ID id, uint32_t freq, int8_t power, uint16_t time);
static int16_t RadioRssi(SX1262_ID id, RadioModems_t modem);
static void RadioWrite(SX1262_ID id, uint16_t addr, uint8_t data);
static uint8_t RadioRead(SX1262_ID id, uint16_t addr);
static void RadioWriteBuffer(SX1262_ID id, uint16_t addr, const uint8_t* const buffer, uint8_t size);
static void RadioReadBuffer(SX1262_ID id, uint16_t addr, uint8_t* const buffer, uint8_t size);
static void RadioSetMaxPayloadLength(SX1262_ID id, RadioModems_t modem, uint8_t max);
static void RadioSetPublicNetwork(SX1262_ID id, bool enable);
static uint32_t RadioGetWakeupTime(SX1262_ID id);
static void RadioBgIrqProcess(SX1262_ID id);
static void RadioIrqProcess(void);
static void RadioRxBoosted(SX1262_ID id, uint32_t timeout);
static void RadioSetRxDutyCycle(SX1262_ID id, uint32_t rxTime, uint32_t sleepTime);
static void RadioModulationParams_Print(SX1262_ID id, const ModulationParams_t* const modParams);
static void RadioPacketParams_Print(SX1262_ID id, const PacketParams_t* const packetParams);
static void LoRaSetModulationParams(uint32_t sf, uint32_t bwIndex, uint32_t cr, ModulationParams_t* const modParams);
static void LoRaSetPacketParams(uint32_t sf, uint32_t preambleLen, bool fixLen, bool crcOn, bool iqInverted, PacketParams_t* const packetParams);
static void GfskSetModulationParams(void);
static void GfskSetPacketParams(void);

/*!
 * Radio driver structure initialization
 */
const Radio_t Radio =
{
    RadioInit,
    RadioReInit,
    RadioGetStatus,
    RadioSetModem,
    RadioSetChannel,
    RadioIsChannelFree,
    RadioRandom,
    RadioSetRxConfig,
    RadioSetTxConfig,
    RadioCheckRfFrequency,
    RadioTimeOnAir,
    RadioSend,
    RadioSleep,
    RadioStandby,
    RadioRx,
    RadioSetCadParams,
    RadioStartCad,
    RadioSetTxContinuousWave,
    RadioRssi,
    RadioWrite,
    RadioRead,
    RadioWriteBuffer,
    RadioReadBuffer,
    RadioSetMaxPayloadLength,
    RadioSetPublicNetwork,
    RadioGetWakeupTime,
    RadioBgIrqProcess,
    RadioIrqProcess,
    /* Available on SX126x only */
    RadioRxBoosted,
    RadioSetRxDutyCycle
};

/*!
 * FSK bandwidth definition
 */
typedef struct
{
    uint32_t bandwidth;
    uint8_t regValue;
} FskBandwidth_t;

/*!
 * Precomputed FSK bandwidth registers values
 */
const FskBandwidth_t FskBandwidths[] = {
    { 4800, 0x1F },
    { 5800, 0x17 },
    { 7300, 0x0F },
    { 9700, 0x1E },
    { 11700, 0x16 },
    { 14600, 0x0E },
    { 19500, 0x1D },
    { 23400, 0x15 },
    { 29300, 0x0D },
    { 39000, 0x1C },
    { 46900, 0x14 },
    { 58600, 0x0C },
    { 78200, 0x1B },
    { 93800, 0x13 },
    { 117300, 0x0B },
    { 156200, 0x1A },
    { 187200, 0x12 },
    { 234300, 0x0A },
    { 312000, 0x19 },
    { 373600, 0x11 },
    { 467000, 0x09 },
    { 500000, 0x00 }, // Invalid Bandwidth
};

const RadioLoRaBandwidths_t m_bandwidths[] = {  LORA_BW_125,
                                                LORA_BW_250, 
                                                LORA_BW_500, 
                                                LORA_BW_062,    /* not supported */
                                                LORA_BW_041,    /* not supported */
                                                LORA_BW_031,    /* not supported */
                                                LORA_BW_020,    /* not supported */
                                                LORA_BW_015,    /* not supported */
                                                LORA_BW_010,    /* not supported */ 
                                                LORA_BW_007     /* not supported */
                                            };

//                                          SF12    SF11    SF10    SF9    SF8    SF7
static double RadioLoRaSymbTime[3][6] = {{32.768, 16.384, 8.192, 4.096, 2.048, 1.024}, // 125 KHz
                                         {16.384, 8.192, 4.096, 2.048, 1.024, 0.512},  // 250 KHz
                                         {8.192, 4.096, 2.048, 1.024, 0.512, 0.256}};  // 500 KHz

uint8_t m_maxPayloadLength = 0xFF;

static uint32_t m_rxTimeout = 0xFFFF;
static bool m_rxContinuous = false;

static PacketStatus_t RadioPktStatus;
static uint8_t RadioRxPayload[255];

static volatile bool m_irqFired[SX1262_ID_NUMBER] = {false, false};

static RadioModems_t m_modem;

/*!
 * @brief DIO 0 IRQ callback for SX1262 #1
 */
//static void IRAM_ATTR RadioOnDioIrqTx(void);

/*!
 * @brief DIO 0 IRQ callback for SX1262 #2
 */
//static void IRAM_ATTR RadioOnDioIrqRx(void);

/*!
 * Holds the current network type for the radio
 */
typedef struct
{
    bool previous;
    bool current;
} RadioPublicNetwork_t;

static RadioPublicNetwork_t m_radioPublicNetwork = {false, false};

/*!
 * Radio callbacks variable
 */
static RadioEvents_t* m_radioEvents;

/*!
 * Radio hardware and global parameters
 */
static sx126xParams_t m_sx1262Params;

/*!
 * Returns the known FSK bandwidth registers value
 *
 * @param  bandwidth Bandwidth value in Hz
 * @retval regValue Bandwidth register value.
 */
static uint8_t RadioGetFskBandwidthRegValue(uint32_t bandwidth)
{
    uint8_t i;

    if (bandwidth == 0)
    {
        return (0x1F);
    }

    for (i = 0; i < (sizeof(FskBandwidths) / sizeof(FskBandwidth_t)) - 1; i++)
    {
        if ((bandwidth >= FskBandwidths[i].bandwidth) && (bandwidth < FskBandwidths[i + 1].bandwidth))
        {
            return FskBandwidths[i + 1].regValue;
        }
    }
    // In case value not found, return bandwidth 0
    return (0x1F);
    // ERROR: Value not found
    // while (1)
    // 	;
}

static void RadioInit(SX1262_ID id, RadioEvents_t* events, const GpioConfig_t* const config)
{
    assert(events);
    assert(id < SX1262_ID_NUMBER);
    assert(config);

    m_radioEvents = events;

    /* board specific GPIO init */
    m_sx1262.GpioInit(id, config);

    //(id) ? m_sx1262.Begin(id, RadioOnDioIrqRx) : m_sx1262.Begin(id, RadioOnDioIrqTx);

    m_sx1262.SetStandby(id, STDBY_RC);
    m_sx1262.SetBufferBaseAddress(id, 0x00, 0x00);
    m_sx1262.SetTxParams(id, 0, RADIO_RAMP_200_US);
    m_sx1262.SetDioIrqParams(id, IRQ_RADIO_ALL, IRQ_RADIO_ALL, IRQ_RADIO_NONE, IRQ_RADIO_NONE);

    m_irqFired[id] = false;
}

static void RadioReInit(SX1262_ID id, RadioEvents_t* events)
{
    assert(events);
    assert(id < SX1262_ID_NUMBER);

    m_radioEvents = events;

    //(id) ? m_sx1262.Begin(id, RadioOnDioIrqRx) : m_sx1262.Begin(id, RadioOnDioIrqTx);

    m_irqFired[id] = false;
}

static RadioState_t RadioGetStatus(SX1262_ID id)
{
    assert(id < SX1262_ID_NUMBER);

    switch (m_sx1262.GetOperatingMode(id))
    {
    case MODE_TX:
        return RF_TX_RUNNING;
    case MODE_RX:
        return RF_RX_RUNNING;
    case MODE_CAD:
        return RF_CAD;
    default:
        return RF_IDLE;
    }
}

static void RadioSetModem(SX1262_ID id, RadioModems_t modem)
{
    assert(id < SX1262_ID_NUMBER);
    assert(modem == MODEM_FSK || modem == MODEM_LORA);

    switch (modem)
    {
    case MODEM_FSK:
        m_sx1262.SetPacketType(id, PACKET_TYPE_GFSK);

        // When switching to GFSK mode the LoRa SyncWord register value is reset
        // Thus, we also reset the RadioPublicNetwork variable
        m_radioPublicNetwork.current = false;
        m_modem = modem;
        break;

    case MODEM_LORA:
        m_sx1262.SetPacketType(id, PACKET_TYPE_LORA);
        // Public/Private network register is reset when switching modems
        //if (m_radioPublicNetwork.current != m_radioPublicNetwork.previous)
        //{
        //    m_radioPublicNetwork.current = m_radioPublicNetwork.previous;
        //    RadioSetPublicNetwork(id, m_radioPublicNetwork.current);
        //}
        m_modem = modem;
        break;
    
    default:
        //Serial.printf("Radio: Fail to set modem type\r\n");
    break;
    }
}

static void RadioSetChannel(SX1262_ID id, uint32_t freq)
{
    assert(id < SX1262_ID_NUMBER);

    //(id) ? Serial.printf("SX1262 RX Frequency: [%d] Hz\r\n", freq) : Serial.printf("SX1262 TX Frequency: [%d] Hz\r\n", freq);

    m_sx1262.SetRfFrequency(id, freq);
}

static bool RadioIsChannelFree(SX1262_ID id, RadioModems_t modem, uint32_t freq, int16_t rssiThresh, uint32_t maxCarrierSenseTime)
{
    assert(id < SX1262_ID_NUMBER);
    assert(modem == MODEM_FSK || modem == MODEM_LORA);

    bool status = true;
    int16_t rssi = 0;
    uint32_t carrierSenseTime = 0;

    if (RadioGetStatus(id) != RF_IDLE)
    {
        return false;
    }

    RadioSetModem(id, modem);

    RadioSetChannel(id, freq);

    RadioRx(id, 0);
#if 0
    //delay(1);

    //carrierSenseTime = TimerGetCurrentTime();

    // Perform carrier sense for maxCarrierSenseTime
    while (TimerGetElapsedTime(carrierSenseTime) < maxCarrierSenseTime)
    {
        rssi = RadioRssi(id, modem);

        if (rssi > rssiThresh)
        {
            status = false;
            break;
        }
    }
    RadioSleep(id);
#endif
    return status;
}

static uint32_t RadioRandom(SX1262_ID id)
{
    assert(id < SX1262_ID_NUMBER);

    uint32_t rnd = 0;

    /*
     * Radio setup for random number generation
     */
    // Set LoRa modem ON
    RadioSetModem(id, MODEM_LORA);

    // Set radio in continuous reception
    m_sx1262.SetRx(id, 0);

    rnd = m_sx1262.GetRandom(id);
    RadioSleep(id);

    return rnd;
}

static void RadioSetRxConfig(   SX1262_ID id,
                                RadioModems_t modem,
                                uint32_t bandwidthIndex,
                                uint32_t datarate,
                                uint8_t coderate,
                                uint32_t bandwidthAfc,
                                uint16_t preambleLen,
                                uint16_t symbTimeout,
                                bool fixLen,
                                uint8_t payloadLen,
                                bool crcOn,
                                bool freqHopOn,
                                uint8_t hopPeriod,
                                bool iqInverted,
                                bool rxContinuous)
{
    assert(id < SX1262_ID_NUMBER);

    //Serial.printf(" ### Radio RX config ###\r\n");

    m_rxContinuous = rxContinuous;

    if (rxContinuous == true)
    {
        symbTimeout = 0;
    }
    if (fixLen)
    {
        m_maxPayloadLength = payloadLen;
    }

    switch (modem)
    {
    case MODEM_FSK:
        assert(bandwidthIndex > 0 && bandwidthIndex < 21);
#if 1
        GfskSetModulationParams();
        GfskSetPacketParams();
#else
        m_sx1262.SetStopRxTimerOnPreambleDetect(id, false);
        m_sx1262Params.ModulationParams.PacketType = PACKET_TYPE_GFSK;
        m_sx1262Params.ModulationParams.Params.Gfsk.BitRate = datarate;
        m_sx1262Params.ModulationParams.Params.Gfsk.ModulationShaping = MOD_SHAPING_G_BT_1;
        m_sx1262Params.ModulationParams.Params.Gfsk.Bandwidth = RadioGetFskBandwidthRegValue(bandwidthIndex);

        m_sx1262Params.PacketParams.PacketType = PACKET_TYPE_GFSK;
        m_sx1262Params.PacketParams.Params.Gfsk.PreambleLength = (preambleLen << 3); // convert byte into bit
        m_sx1262Params.PacketParams.Params.Gfsk.PreambleMinDetect = RADIO_PREAMBLE_DETECTOR_08_BITS;
        m_sx1262Params.PacketParams.Params.Gfsk.SyncWordLength = 3 << 3; // convert byte into bit
        m_sx1262Params.PacketParams.Params.Gfsk.AddrComp = RADIO_ADDRESSCOMP_FILT_OFF;
        m_sx1262Params.PacketParams.Params.Gfsk.HeaderType = (fixLen == true) ? RADIO_PACKET_FIXED_LENGTH : RADIO_PACKET_VARIABLE_LENGTH;
        m_sx1262Params.PacketParams.Params.Gfsk.PayloadLength = MaxPayloadLength;
        if (crcOn == true)
        {
            m_sx1262Params.PacketParams.Params.Gfsk.CrcLength = RADIO_CRC_2_BYTES_CCIT;
        }
        else
        {
            m_sx1262Params.PacketParams.Params.Gfsk.CrcLength = RADIO_CRC_OFF;
        }
        m_sx1262Params.PacketParams.Params.Gfsk.DcFree = RADIO_DC_FREEWHITENING;
#endif

        RadioStandby(id);
        RadioSetModem(id, (m_sx1262Params.ModulationParams.PacketType == PACKET_TYPE_GFSK) ? MODEM_FSK : MODEM_LORA);
        m_sx1262.SetModulationParams(id, &m_sx1262Params.ModulationParams);
        m_sx1262.SetPacketParams(id, &m_sx1262Params.PacketParams);
        uint8_t syncWord[8];
        syncWord[0] = 0xC1;
        syncWord[1] = 0x94;
        syncWord[2] = 0xC1;
        syncWord[3] = 0x00;
        syncWord[4] = 0x00;
        syncWord[5] = 0x00;
        syncWord[6] = 0x00;
        syncWord[7] = 0x00;
        m_sx1262.SetFskSyncWord(id, syncWord);
        m_sx1262.SetFskWhiteningSeed(id, 0x01FF);

        m_rxTimeout = (uint32_t)(symbTimeout * ((1.0 / (double)datarate) * 8.0) * 1000);
        break;

    case MODEM_LORA:
        assert(bandwidthIndex >= 0 && bandwidthIndex <= 2);

        m_sx1262.SetStopRxTimerOnPreambleDetect(id, false);
        m_sx1262.SetLoRaSymbNumTimeout(id, symbTimeout);

        LoRaSetModulationParams(datarate, bandwidthIndex, coderate, &m_sx1262Params.ModulationParams);
        LoRaSetPacketParams(datarate, preambleLen, fixLen, crcOn, iqInverted, &m_sx1262Params.PacketParams);

        RadioStandby(id);
        RadioSetModem(id, MODEM_LORA);
        
        m_sx1262.SetModulationParams(id, &m_sx1262Params.ModulationParams);
        m_sx1262.SetPacketParams(id, &m_sx1262Params.PacketParams);
        m_sx1262.SetLoRaSymbNumTimeout(id, symbTimeout);

/* WORKAROUND */
        m_sx1262.Workaround_InvertedIqOpertation(id, m_sx1262Params.PacketParams.Params.LoRa.InvertIQ);
/* WORKAROUND */

        break;
    }

    RadioModulationParams_Print(id, &m_sx1262Params.ModulationParams);
    RadioPacketParams_Print(id, &m_sx1262Params.PacketParams);
}

static void RadioSetTxConfig(   SX1262_ID id,
                                RadioModems_t modem,
                                int8_t power,
                                uint32_t fdev,
                                uint32_t bandwidthIndex,
                                uint32_t datarate,
                                uint8_t coderate,
                                uint16_t preambleLen,
                                bool fixLen,
                                bool crcOn,
                                bool freqHopOn,
                                uint8_t hopPeriod,
                                bool iqInverted,
                                uint32_t timeout)
{
    assert(id < SX1262_ID_NUMBER);

    //Serial.printf(" ### Radio TX config ###\r\n");

    switch (modem)
    {
    case MODEM_FSK:
        assert(bandwidthIndex > 0 && bandwidthIndex < 21);
#if 1
        GfskSetModulationParams();
        GfskSetPacketParams();
#else
        m_sx1262Params.ModulationParams.PacketType = PACKET_TYPE_GFSK;
        m_sx1262Params.ModulationParams.Params.Gfsk.BitRate = datarate;

        m_sx1262Params.ModulationParams.Params.Gfsk.ModulationShaping = MOD_SHAPING_G_BT_1;
        m_sx1262Params.ModulationParams.Params.Gfsk.Bandwidth = RadioGetFskBandwidthRegValue(bandwidthIndex);
        m_sx1262Params.ModulationParams.Params.Gfsk.Fdev = fdev;

        m_sx1262Params.PacketParams.PacketType = PACKET_TYPE_GFSK;
        m_sx1262Params.PacketParams.Params.Gfsk.PreambleLength = (preambleLen << 3); // convert byte into bit
        m_sx1262Params.PacketParams.Params.Gfsk.PreambleMinDetect = RADIO_PREAMBLE_DETECTOR_08_BITS;
        m_sx1262Params.PacketParams.Params.Gfsk.SyncWordLength = 3 << 3; // convert byte into bit
        m_sx1262Params.PacketParams.Params.Gfsk.AddrComp = RADIO_ADDRESSCOMP_FILT_OFF;
        m_sx1262Params.PacketParams.Params.Gfsk.HeaderType = (fixLen == true) ? RADIO_PACKET_FIXED_LENGTH : RADIO_PACKET_VARIABLE_LENGTH;

        if (crcOn == true)
        {
            m_sx1262Params.PacketParams.Params.Gfsk.CrcLength = RADIO_CRC_2_BYTES_CCIT;
        }
        else
        {
            m_sx1262Params.PacketParams.Params.Gfsk.CrcLength = RADIO_CRC_OFF;
        }
        m_sx1262Params.PacketParams.Params.Gfsk.DcFree = RADIO_DC_FREEWHITENING;
#endif

        RadioStandby(id);
        RadioSetModem(id, modem);
        
        m_sx1262.SetModulationParams(id, &m_sx1262Params.ModulationParams);
        m_sx1262.SetPacketParams(id, &m_sx1262Params.PacketParams);
        uint8_t syncWord[8];
        syncWord[0] = 0xC1;
        syncWord[1] = 0x94;
        syncWord[2] = 0xC1;
        syncWord[3] = 0x00;
        syncWord[4] = 0x00;
        syncWord[5] = 0x00;
        syncWord[6] = 0x00;
        syncWord[7] = 0x00;
        m_sx1262.SetFskSyncWord(id, syncWord);
        m_sx1262.SetFskWhiteningSeed(id, 0x01FF);
        break;

    case MODEM_LORA:
        assert(bandwidthIndex >= 0 && bandwidthIndex <= 2);
        assert(coderate >= 1 && coderate <= 4);

        LoRaSetModulationParams(datarate, bandwidthIndex, coderate, &m_sx1262Params.ModulationParams);
        LoRaSetPacketParams(datarate, preambleLen, fixLen, crcOn, iqInverted, &m_sx1262Params.PacketParams);

        RadioStandby(id);
        RadioSetModem(id, modem);

        m_sx1262.SetModulationParams(id, &m_sx1262Params.ModulationParams);
        m_sx1262.SetPacketParams(id, &m_sx1262Params.PacketParams);
        break;
    }
/* WORKAROUND */
    m_sx1262.Workaround_LoRaBW500(id, m_sx1262Params.ModulationParams.Params.LoRa.Bandwidth);
/* WORKAROUND */

    m_sx1262.SetRfTxPower(id, power);

    RadioModulationParams_Print(id, &m_sx1262Params.ModulationParams);
    RadioPacketParams_Print(id, &m_sx1262Params.PacketParams);
}

bool RadioCheckRfFrequency(SX1262_ID id, uint32_t frequency)
{
/* NOTE: TODO: */
    return true;
}

static uint32_t RadioTimeOnAir(SX1262_ID id, RadioModems_t modem, uint8_t pktLen)
{
    uint32_t airTime = 0;

    switch (modem)
    {
    case MODEM_FSK:
    {
        // CRC Length calculation, catering for each type of CRC Calc offered in libary
        uint8_t crcLength = (uint8_t)(m_sx1262Params.PacketParams.Params.Gfsk.CrcLength);
        if ((crcLength == RADIO_CRC_2_BYTES) || (crcLength == RADIO_CRC_2_BYTES_INV) || (crcLength == RADIO_CRC_2_BYTES_IBM) || (crcLength == RADIO_CRC_2_BYTES_CCIT))
        {
            crcLength = 2;
        }
        else if ((crcLength == RADIO_CRC_1_BYTES) || (crcLength == RADIO_CRC_1_BYTES_INV))
        {
            crcLength = 1;
        }
        else
        {
            crcLength = 0;
        }
        //airTime = rint((8 * (m_sx1262Params.PacketParams.Params.Gfsk.PreambleLength + (m_sx1262Params.PacketParams.Params.Gfsk.SyncWordLength >> 3) + ((m_sx1262Params.PacketParams.Params.Gfsk.HeaderType == RADIO_PACKET_FIXED_LENGTH) ? 0.0 : 1.0) + pktLen + (crcLength)) /
        //                m_sx1262Params.ModulationParams.Params.Gfsk.BitRate) *
        //               1e3);
    }
    break;
    case MODEM_LORA:
    {
        double ts = RadioLoRaSymbTime[m_sx1262Params.ModulationParams.Params.LoRa.Bandwidth - 4][12 - m_sx1262Params.ModulationParams.Params.LoRa.SpreadingFactor];
        // time of preamble
        double tPreamble = (m_sx1262Params.PacketParams.Params.LoRa.PreambleLength + 4.25) * ts;
        // Symbol length of payload and time
        //double tmp = ceil((8 * pktLen - 4 * m_sx1262Params.ModulationParams.Params.LoRa.SpreadingFactor +
        //                   28 + 16 * m_sx1262Params.PacketParams.Params.LoRa.CrcMode -
        //                   ((m_sx1262Params.PacketParams.Params.LoRa.HeaderType == LORA_PACKET_FIXED_LENGTH) ? 20 : 0)) /
        //                  (double)(4 * (m_sx1262Params.ModulationParams.Params.LoRa.SpreadingFactor -
        //                                ((m_sx1262Params.ModulationParams.Params.LoRa.LowDatarateOptimize > 0) ? 2 : 0)))) *
        //             ((m_sx1262Params.ModulationParams.Params.LoRa.CodingRate % 4) + 4);
        //double nPayload = 8 + ((tmp > 0) ? tmp : 0);
        //double tPayload = nPayload * ts;
        // Time on air
        //double tOnAir = tPreamble + tPayload;
        // return milli seconds
        //airTime = floor(tOnAir + 0.999);
    }
    break;
    }
    return airTime;
}

static void RadioSend(SX1262_ID id, const uint8_t* const buffer, uint8_t size)
{
    assert(id < SX1262_ID_NUMBER);

    m_sx1262.SetDioIrqParams(id, IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT,
                            IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT,
                            IRQ_RADIO_NONE,
                            IRQ_RADIO_NONE);

/* NOTE: do we need to change packet param on fly ??? */
#if 0
    if (m_sx1262.GetPacketType(id) == PACKET_TYPE_LORA)
    {
        m_sx1262Params.PacketParams.Params.LoRa.PayloadLength = size;
    }
    else
    {
        m_sx1262Params.PacketParams.Params.Gfsk.PayloadLength = size;
    }
#endif

    m_sx1262.SetPacketParams(id, &m_sx1262Params.PacketParams);

    m_sx1262.SendPayload(id, buffer, size, 0);
}

static void RadioSleep(SX1262_ID id)
{
    assert(id < SX1262_ID_NUMBER);

    SleepParams_t params = {0};

    params.Fields.WarmStart = 1;

    m_sx1262.SetSleep(id, &params);

    //delay(2);
}

static void RadioStandby(SX1262_ID id)
{
    assert(id < SX1262_ID_NUMBER);

    m_sx1262.SetStandby(id, STDBY_RC);
}

static void RadioRx(SX1262_ID id, uint32_t timeout)
{
    assert(id < SX1262_ID_NUMBER);
    
    m_sx1262.SetDioIrqParams(id, IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_HEADER_ERROR | IRQ_CRC_ERROR,
                          IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_HEADER_ERROR | IRQ_CRC_ERROR,
                          IRQ_RADIO_NONE,
                          IRQ_RADIO_NONE);

    //Serial.printf("RADIO RX window timeout: [%d] ms\r\n", timeout);
    // Even Continous mode is selected, put a timeout here
    if (timeout != 0)
    {
#if 0
        TimerSetValue(&RxTimeoutTimer, timeout);
        TimerStart(&RxTimeoutTimer);
#endif
    }
    //Serial.printf("Continuous RX: [%s]\r\n", m_rxContinuous ? "ENABLED" : "DISABLED");
    if (m_rxContinuous == true)
    {
        m_sx1262.SetRx(id, 0xFFFFFF); /* Rx Continuous */
    }
    else
    {
        m_sx1262.SetRx(id, timeout << 6);
    }
}

static void RadioRxBoosted(SX1262_ID id, uint32_t timeout)
{
    assert(id < SX1262_ID_NUMBER);

    m_sx1262.SetDioIrqParams(id, IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_HEADER_ERROR | IRQ_CRC_ERROR,
                          IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_HEADER_ERROR | IRQ_CRC_ERROR,
                          IRQ_RADIO_NONE,
                          IRQ_RADIO_NONE);

    if (m_rxContinuous == true)
    {
        // Even Continous mode is selected, put a timeout here
        if (timeout != 0)
        {
#if 0
            TimerSetValue(&RxTimeoutTimer, timeout);
            TimerStart(&RxTimeoutTimer);
#endif
        }
        m_sx1262.SetRxBoosted(id, 0xFFFFFF);    /* Rx Continuous */
    }
    else
    {
        m_sx1262.SetRxBoosted(id, timeout << 6);
    }
}

static void RadioSetRxDutyCycle(SX1262_ID id, uint32_t rxTime, uint32_t sleepTime)
{
    assert(id < SX1262_ID_NUMBER);

    m_sx1262.SetDioIrqParams(id, IRQ_RADIO_ALL | IRQ_RX_TX_TIMEOUT,
                          IRQ_RADIO_ALL | IRQ_RX_TX_TIMEOUT,
                          IRQ_RADIO_NONE, IRQ_RADIO_NONE);

    m_sx1262.SetRxDutyCycle(id, rxTime, sleepTime);
}

static void RadioSetCadParams(SX1262_ID id, uint8_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin, uint8_t cadExitMode, uint32_t cadTimeout)
{
    m_sx1262.SetCadParams(id, (RadioLoRaCadSymbols_t)cadSymbolNum, cadDetPeak, cadDetMin, (RadioCadExitModes_t)cadExitMode, cadTimeout);
}

static void RadioStartCad(SX1262_ID id)
{
    assert(id < SX1262_ID_NUMBER);

    m_sx1262.SetDioIrqParams(id, IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED,
                          IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED,
                          IRQ_RADIO_NONE, IRQ_RADIO_NONE);

    m_sx1262.SetCad(id);
}

static void RadioSetTxContinuousWave(SX1262_ID id, uint32_t freq, int8_t power, uint16_t time)
{
    assert(id < SX1262_ID_NUMBER);

    m_sx1262.SetRfFrequency(id, freq);
    m_sx1262.SetRfTxPower(id, power);
    m_sx1262.SetTxContinuousWave(id);
}

static int16_t RadioRssi(SX1262_ID id, RadioModems_t modem)
{
    assert(id < SX1262_ID_NUMBER);

    return m_sx1262.GetRssiInst(id);
}

static void RadioWrite(SX1262_ID id, uint16_t addr, uint8_t data)
{
    assert(id < SX1262_ID_NUMBER);

    m_sx1262.WriteRegister(id, addr, data);
}

static uint8_t RadioRead(SX1262_ID id, uint16_t addr)
{
    assert(id < SX1262_ID_NUMBER);

    return m_sx1262.ReadRegister(id, addr);
}

static void RadioWriteBuffer(SX1262_ID id, uint16_t addr, const uint8_t* const buffer, uint8_t size)
{
    assert(id < SX1262_ID_NUMBER);

    m_sx1262.WriteRegisters(id, addr, buffer, size);
}

static void RadioReadBuffer(SX1262_ID id, uint16_t addr, uint8_t* const buffer, uint8_t size)
{
    assert(id < SX1262_ID_NUMBER);

    m_sx1262.ReadRegisters(id, addr, buffer, size);
}

static void RadioSetMaxPayloadLength(SX1262_ID id, RadioModems_t modem, uint8_t max)
{
    assert(id < SX1262_ID_NUMBER);

    if (modem == MODEM_LORA)
    {
        m_sx1262Params.PacketParams.Params.LoRa.PayloadLength = m_maxPayloadLength = max;
        //Serial.printf("LoRa max. payload len: [%d] bytes \r\n", m_sx1262Params.PacketParams.Params.LoRa.PayloadLength);
        m_sx1262.SetPacketParams(id, &m_sx1262Params.PacketParams);
    }
    else
    {
        if (m_sx1262Params.PacketParams.Params.Gfsk.HeaderType == RADIO_PACKET_VARIABLE_LENGTH)
        {
            m_sx1262Params.PacketParams.Params.Gfsk.PayloadLength = m_maxPayloadLength = max;
            m_sx1262.SetPacketParams(id, &m_sx1262Params.PacketParams);
        }
    }
}

static void RadioSetPublicNetwork(SX1262_ID id, bool enable)
{
    assert(id < SX1262_ID_NUMBER);

    m_radioPublicNetwork.current = m_radioPublicNetwork.previous = enable;

    if (enable == true)
    {
        /* Change LoRa modem SyncWord */
        m_sx1262.WriteRegister(id, SX126X_REG_LORA_SYNC_WORD_MSB, (SX126X_LORA_SYNC_WORD_PUBLIC >> 8) & 0xFF);
        m_sx1262.WriteRegister(id, SX126X_REG_LORA_SYNC_WORD_MSB + 1, SX126X_LORA_SYNC_WORD_PUBLIC & 0xFF);
    }
    else
    {
        /* Change LoRa modem SyncWord */
        m_sx1262.WriteRegister(id, SX126X_REG_LORA_SYNC_WORD_MSB, (SX126X_LORA_SYNC_WORD_PRIVATE >> 8) & 0xFF);
        m_sx1262.WriteRegister(id, SX126X_REG_LORA_SYNC_WORD_MSB + 1, SX126X_LORA_SYNC_WORD_PRIVATE & 0xFF);
    }
}

static uint32_t RadioGetWakeupTime(SX1262_ID id)
{
    assert(id < SX1262_ID_NUMBER);

    return (m_sx1262.GetTcxoWakeupTime() + RADIO_WAKEUP_TIME);
}

#if 0
static void IRAM_ATTR RadioOnDioIrqTx(void)
{
    m_irqFired[SX1262_ID_TX] = true;
}

static void IRAM_ATTR RadioOnDioIrqRx(void)
{
    m_irqFired[SX1262_ID_RX] = true;
}
#endif

static void RadioBgIrqProcess(SX1262_ID id)
{
    assert(id < SX1262_ID_NUMBER);

    if (m_irqFired[id])
    {
        m_irqFired[id] = false;

        uint16_t irqRegs = m_sx1262.GetIrqStatus(id);
        m_sx1262.ClearIrqStatus(id, irqRegs);

        if ((irqRegs & IRQ_TX_DONE) == IRQ_TX_DONE)
        {
            //Serial.printf("IRQ: RADIO IRQ_TX_DONE\r\n");

            //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
            m_sx1262.SetOperatingMode(id, MODE_STDBY_RC);
            if ((m_radioEvents != NULL) && (m_radioEvents->TxDone != NULL))
            {
                (*m_radioEvents->TxDone)(id);
            }
        }

        if ((irqRegs & IRQ_RX_DONE) == IRQ_RX_DONE)
        {
            //Serial.printf("IRQ: RADIO IRQ_RX_DONE\r\n");
            uint8_t size;

            if (m_rxContinuous == false)
            {
                //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
                m_sx1262.SetOperatingMode(id, MODE_STDBY_RC);

/* WORKAROUND */
                m_sx1262.Workaround_ImplicitHeaderModeTimeout(id);
/* WORKAROUND */      
            }
            memset(RadioRxPayload, 0, 255);

            if ((irqRegs & IRQ_CRC_ERROR) == IRQ_CRC_ERROR)
            {
                //Serial.printf("IRQ: RADIO IRQ_CRC_ERROR\r\n");

                uint8_t size;
                // Discard buffer
                memset(RadioRxPayload, 0, 255);
                m_sx1262.GetPayload(id, RadioRxPayload, &size, 255);
                m_sx1262.GetPacketStatus(id, &RadioPktStatus);
                if ((m_radioEvents != NULL) && (m_radioEvents->RxError))
                {
                    (*m_radioEvents->RxError)(id);
                }
            }
            else
            {
                m_sx1262.GetPayload(id, RadioRxPayload, &size, 255);
                m_sx1262.GetPacketStatus(id, &RadioPktStatus);
                if ((m_radioEvents != NULL) && (m_radioEvents->RxDone != NULL))
                {
                    (*m_radioEvents->RxDone)(id, RadioRxPayload, size, RadioPktStatus.Params.LoRa.RssiPkt, RadioPktStatus.Params.LoRa.SnrPkt);
                }
            }
        }

        if ((irqRegs & IRQ_CAD_DONE) == IRQ_CAD_DONE)
        {
            //Serial.printf("IRQ: RADIO IRQ_CAD_DONE\r\n");
            //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
            m_sx1262.SetOperatingMode(id, MODE_STDBY_RC);
            if ((m_radioEvents != NULL) && (m_radioEvents->CadDone != NULL))
            {
                (*m_radioEvents->CadDone)(id, ((irqRegs & IRQ_CAD_ACTIVITY_DETECTED) == IRQ_CAD_ACTIVITY_DETECTED));
            }
        }

        if ((irqRegs & IRQ_RX_TX_TIMEOUT) == IRQ_RX_TX_TIMEOUT)
        {
            if (m_sx1262.GetOperatingMode(id) == MODE_TX)
            {
                //Serial.printf("IRQ: RADIO IRQ_TX_TIMEOUT\r\n");

                //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
                m_sx1262.SetOperatingMode(id, MODE_STDBY_RC);
                if ((m_radioEvents != NULL) && (m_radioEvents->TxTimeout != NULL))
                {
                    (*m_radioEvents->TxTimeout)(id);
                }
            }
            else if (m_sx1262.GetOperatingMode(id) == MODE_RX)
            {
                //Serial.printf("IRQ: RADIO IRQ_RX_TIMEOUT\r\n");

                //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
                m_sx1262.SetOperatingMode(id, MODE_STDBY_RC);
                if ((m_radioEvents != NULL) && (m_radioEvents->RxTimeout != NULL))
                {
                    (*m_radioEvents->RxTimeout)(id);
                }
            }
        }

        if ((irqRegs & IRQ_PREAMBLE_DETECTED) == IRQ_PREAMBLE_DETECTED)
        {
            //Serial.printf("IRQ: RADIO IRQ_PREAMBLE_DETECTED\r\n");
            if ((m_radioEvents != NULL) && (m_radioEvents->PreAmpDetect != NULL))
            {
                (*m_radioEvents->PreAmpDetect)(id);
            }
        }

        if ((irqRegs & IRQ_SYNCWORD_VALID) == IRQ_SYNCWORD_VALID)
        {
            //__NOP( );
        }

        if ((irqRegs & IRQ_HEADER_VALID) == IRQ_HEADER_VALID)
        {
            //__NOP( );
        }

        if ((irqRegs & IRQ_HEADER_ERROR) == IRQ_HEADER_ERROR)
        {
            //Serial.printf("IRQ: RADIO IRQ_HEADER_ERROR\r\n");

            if (m_rxContinuous == false)
            {
                //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
                m_sx1262.SetOperatingMode(id, MODE_STDBY_RC);
            }
            if ((m_radioEvents != NULL) && (m_radioEvents->RxError != NULL))
            {
                (*m_radioEvents->RxError)(id);
            }
        }
    }
}

static void RadioIrqProcess(void)
{
    for (uint8_t index = 0; index < SX1262_ID_NUMBER; index++)
    {
        RadioBgIrqProcess((SX1262_ID)index);    
    }
}

static void RadioModulationParams_Print(SX1262_ID id, const ModulationParams_t* const modParams)
{
    assert(id < SX1262_ID_NUMBER);
    assert(modParams);
#if 0
    Serial.printf(" ### Radio modulation params\r\n");
    Serial.printf("Radio ID: [%s]\r\n", (id) ? "SX1262_ID_RX" : "SX1262_ID_TX");
    Serial.printf("Modem type: [%s]\r\n", (modParams->PacketType) ? "PACKET_TYPE_LORA" : "PACKET_TYPE_GFSK");

    switch (modParams->PacketType)
    {
    case PACKET_TYPE_GFSK:
        Serial.printf("Gfsk Bandwidth: [%d]\r\n", modParams->Params.Gfsk.Bandwidth);
        Serial.printf("Gfsk BitRate: [%d]\r\n", modParams->Params.Gfsk.BitRate);
        Serial.printf("Gfsk Fdev: [%d]\r\n", modParams->Params.Gfsk.Fdev);
        Serial.printf("Gfsk ModulationShaping: [%d]\r\n", modParams->Params.Gfsk.ModulationShaping);
        break;

    case PACKET_TYPE_LORA:
        Serial.printf("LoRa Bandwidth: [%s%s%s]\r\n",   (modParams->Params.LoRa.Bandwidth == LORA_BW_500) ? "LORA_BW_500" : "", 
                                                        (modParams->Params.LoRa.Bandwidth == LORA_BW_250) ? "LORA_BW_250" : "",
                                                        (modParams->Params.LoRa.Bandwidth == LORA_BW_125) ? "LORA_BW_125" : ""
                                                        );
        Serial.printf("LoRa CodingRate: [%s%s%s%s]\r\n",    (modParams->Params.LoRa.CodingRate == LORA_CR_4_5) ? "LORA_CR_4_5" : "",
                                                            (modParams->Params.LoRa.CodingRate == LORA_CR_4_6) ? "LORA_CR_4_6" : "",
                                                            (modParams->Params.LoRa.CodingRate == LORA_CR_4_7) ? "LORA_CR_4_7" : "",
                                                            (modParams->Params.LoRa.CodingRate == LORA_CR_4_8) ? "LORA_CR_4_8" : ""
                                                        );
        Serial.printf("LoRa LowDatarateOptimize: [%d]\r\n", modParams->Params.LoRa.LowDatarateOptimize);
        Serial.printf("LoRa SpreadingFactor: [%s%s%s%s%s%s]\r\n",   (modParams->Params.LoRa.SpreadingFactor == LORA_SF7) ? "LORA_SF7" : "",
                                                                    (modParams->Params.LoRa.SpreadingFactor == LORA_SF8) ? "LORA_SF8" : "",
                                                                    (modParams->Params.LoRa.SpreadingFactor == LORA_SF9) ? "LORA_SF9" : "",
                                                                    (modParams->Params.LoRa.SpreadingFactor == LORA_SF10) ? "LORA_SF10" : "",
                                                                    (modParams->Params.LoRa.SpreadingFactor == LORA_SF11) ? "LORA_SF11" : "",
                                                                    (modParams->Params.LoRa.SpreadingFactor == LORA_SF12) ? "LORA_SF12" : ""        
                                                            );
        break;
    
    default:
        Serial.printf("Invalid SX1262 ID\r\n");
        assert(0);
        break;
    }
#endif
}

static void RadioPacketParams_Print(SX1262_ID id, const PacketParams_t* const packetParams)
{
    assert(id < SX1262_ID_NUMBER);
    assert(packetParams);
#if 0
    Serial.printf("Radio ID: [%s]\r\n", (id) ? "SX1262_ID_RX" : "SX1262_ID_TX");
    Serial.printf("Packet type: [%s]\r\n", (packetParams->PacketType) ? "PACKET_TYPE_LORA" : "PACKET_TYPE_GFSK");

    switch (packetParams->PacketType)
    {
    case PACKET_TYPE_GFSK:
        Serial.printf("Gfsk AddrComp: [%d]\r\n", packetParams->Params.Gfsk.AddrComp);
        Serial.printf("Gfsk CrcLength: [%d]\r\n", packetParams->Params.Gfsk.CrcLength);
        Serial.printf("Gfsk DcFree: [%d]\r\n", packetParams->Params.Gfsk.DcFree);
        Serial.printf("Gfsk HeaderType: [%d]\r\n", packetParams->Params.Gfsk.HeaderType);
        Serial.printf("Gfsk PayloadLength: [%d]\r\n", packetParams->Params.Gfsk.PayloadLength);
        Serial.printf("Gfsk PreambleLength: [%d]\r\n", packetParams->Params.Gfsk.PreambleLength);
        Serial.printf("Gfsk PreambleMinDetect: [%d]\r\n", packetParams->Params.Gfsk.PreambleMinDetect);
        Serial.printf("Gfsk SyncWordLength: [%d]\r\n", packetParams->Params.Gfsk.SyncWordLength);
        break;

    case PACKET_TYPE_LORA:
        Serial.printf("LoRa CrcMode: [%s]\r\n", (packetParams->Params.LoRa.CrcMode) ? "LORA_CRC_ON" : "LORA_CRC_OFF");
        Serial.printf("LoRa HeaderType: [%s]\r\n", (packetParams->Params.LoRa.HeaderType) ? "LORA_PACKET_IMPLICIT" : "LORA_PACKET_EXPLICIT");
        Serial.printf("LoRa InvertIQ: [%s]\r\n", (packetParams->Params.LoRa.InvertIQ) ? "LORA_IQ_INVERTED" : "LORA_IQ_NORMAL");
        Serial.printf("LoRa PayloadLength: [%d] bytes\r\n", packetParams->Params.LoRa.PayloadLength);
        Serial.printf("LoRa PreambleLength: [%d] bytes\r\n", packetParams->Params.LoRa.PreambleLength);
        break;
    
    default:
        Serial.printf("Invalid SX1262 ID\r\n");
        assert(0);
        break;
    }
#endif
}

static void LoRaSetModulationParams(uint32_t sf, uint32_t bwIndex, uint32_t cr, ModulationParams_t* const modParams)
{
    assert(modParams);
    
    modParams->PacketType = PACKET_TYPE_LORA;
    modParams->Params.LoRa.SpreadingFactor = (RadioLoRaSpreadingFactors_t)sf;
    modParams->Params.LoRa.Bandwidth = m_bandwidths[bwIndex];
    modParams->Params.LoRa.CodingRate = (RadioLoRaCodingRates_t)cr;

    if (((bwIndex == 0) && ((sf == LORA_SF11) || (sf == LORA_SF12))) || ((bwIndex == 1) && (sf == LORA_SF12)))
    {
        modParams->Params.LoRa.LowDatarateOptimize = 0x01;
    }
    else
    {
        modParams->Params.LoRa.LowDatarateOptimize = 0x00;
    }
}

static void LoRaSetPacketParams(uint32_t sf, uint32_t preambleLen, bool fixLen, bool crcOn, bool iqInverted, PacketParams_t* const packetParams)
{
    assert(packetParams);

    packetParams->PacketType = PACKET_TYPE_LORA;

    if ((sf == LORA_SF5) || (sf == LORA_SF6))
    {
        if (preambleLen < 12)
        {
            packetParams->Params.LoRa.PreambleLength = 12;
        }
        else
        {
            packetParams->Params.LoRa.PreambleLength = preambleLen;
        }
    }
    else
    {
        packetParams->Params.LoRa.PreambleLength = preambleLen;
    }

    packetParams->Params.LoRa.HeaderType = (RadioLoRaPacketLengthsMode_t)fixLen;

    packetParams->Params.LoRa.PayloadLength = m_maxPayloadLength;
    packetParams->Params.LoRa.CrcMode = (RadioLoRaCrcModes_t)crcOn;
    packetParams->Params.LoRa.InvertIQ = (RadioLoRaIQModes_t)iqInverted;
}

static void GfskSetModulationParams(void)
{
/* TOOD: */
}

static void GfskSetPacketParams(void)
{
/* TOOD: */
}
#endif
