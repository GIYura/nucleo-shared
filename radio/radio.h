#ifndef RADIO_H
#define RADIO_H


#if 0
#include "sx126x.h"

/*!
 * Radio driver supported modems
 */
typedef enum
{
     MODEM_FSK = 0,
     MODEM_LORA,
} RadioModems_t;

/*!
 * Radio driver internal state machine states definition
 */
typedef enum
{
     RF_IDLE = 0,   /* The radio is idle */
     RF_RX_RUNNING, /* The radio is in reception state */
     RF_TX_RUNNING, /* The radio is in transmission state */
     RF_CAD,        /* The radio is doing channel activity detection */
} RadioState_t;

/*!
 * \brief Radio driver callback functions
 */
typedef struct
{
     /*!
     * \brief Tx Done callback prototype.
     * \param id - SX1262 ID
     */
     void (*TxDone)(SX1262_ID id);
     /*!
     * \brief Tx Timeout callback prototype.
     */
     void (*TxTimeout)(SX1262_ID id);
     /*!
     * \brief Rx Done callback prototype.
     *
     * \param id - SX1262 ID
     * \param payload - Received buffer pointer
     * \param size - Received buffer size
     * \param rssi - RSSI value computed while receiving the frame [dBm]
     * \param snr - SNR value computed while receiving the frame [dB]
     *                     FSK : N/A ( set to 0 )
     *                     LoRa: SNR value in dB
     */
     void (*RxDone)(SX1262_ID id, uint8_t* const payload, uint16_t size, int16_t rssi, int8_t snr);
     /*!
     * \brief Rx Timeout callback prototype.
     * \param id - SX1262 ID
     */
     void (*RxTimeout)(SX1262_ID id);
     /*!
     * \brief Rx Error callback prototype.
     * \param id - SX1262 ID
     */
     void (*RxError)(SX1262_ID id);
     /*!
     * \brief Preamble detected callback prototype.
     */
     void (*PreAmpDetect)(SX1262_ID id);
     /*!
     * \brief FHSS Change Channel callback prototype.
     * \param id - SX1262 ID
     * \param currentChannel - Index number of the current channel
     */
     void (*FhssChangeChannel)(SX1262_ID id, uint8_t currentChannel);
     /*!
     * \brief CAD Done callback prototype.
     * \param id - SX1262 ID
     * \param channelDetected - Channel Activity detected during the CAD
     */
     void (*CadDone)(SX1262_ID id, bool channelActivityDetected);
} RadioEvents_t;

/*!
 * \brief Radio driver definition
 */
typedef struct
{
	/*!
     * \brief Initializes the radio
     * \param id - SX1262 ID
     * \param events Structure containing the driver callback functions
     * \param config - SX1262 GPIO configuraton
     */
	void (*Init)(SX1262_ID id, RadioEvents_t* events, const GpioConfig_t* const config);
	/*!
     * \brief Re-Initializes the radio after CPU wakeup from deep sleep
     * \param id - SX1262 ID
     * \param events - Structure containing the driver callback functions
     */
	void (*ReInit)(SX1262_ID id, RadioEvents_t* events);
	/*!
     * Return current radio status
     * \param id - SX1262 ID
     * \retval status Radio status.[RF_IDLE, RF_RX_RUNNING, RF_TX_RUNNING]
     */
	RadioState_t (*GetStatus)(SX1262_ID id);
	/*!
     * \brief Configures the radio with the given modem
     * \param id - SX1262 ID
     * \param modem Modem to be used [0: FSK, 1: LoRa]
     */
	void (*SetModem)(SX1262_ID id, RadioModems_t modem);
	/*!
     * \brief Sets the channel frequency
     * \param id - SX1262 ID
     * \param freq - Channel RF frequency
     */
	void (*SetChannel)(SX1262_ID id, uint32_t freq);
	/*!
     * \brief Checks if the channel is free for the given time
     * \param id - SX1262 ID
     * \param modem - Radio modem to be used [0: FSK, 1: LoRa]
     * \param freq - Channel RF frequency
     * \param rssiThresh - RSSI threshold
     * \param maxCarrierSenseTime - Max time while the RSSI is measured
     *
     * \retval isFree         [true: Channel is free, false: Channel is not free]
     */
	bool (*IsChannelFree)(SX1262_ID id, RadioModems_t modem, uint32_t freq, int16_t rssiThresh, uint32_t maxCarrierSenseTime);
	/*!
     * \brief Generates a 32 bits random value based on the RSSI readings
     * \param id - SX1262 ID
     * \remark This function sets the radio in LoRa modem mode and disables
     *         all interrupts.
     *         After calling this function either Radio.SetRxConfig or
     *         Radio.SetTxConfig functions must be called.
     *
     * \retval randomValue 32 bits random value
     */
	uint32_t (*Random)(SX1262_ID id);
	/*!
     * \brief Sets the reception parameters
     * \param id - SX1262 ID
     * \param modem        Radio modem to be used [0: FSK, 1: LoRa]
     * \param bandwidth    Sets the bandwidth
     *                          FSK : >= 2600 and <= 250000 Hz
     *                          LoRa: [0: 125 kHz, 1: 250 kHz,
     *                                 2: 500 kHz, 3: Reserved]
     * \param datarate     Sets the Datarate
     *                          FSK : 600..300000 bits/s
     *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
     *                                10: 1024, 11: 2048, 12: 4096  chips]
     * \param coderate     Sets the coding rate (LoRa only)
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     * \param bandwidthAfc Sets the AFC Bandwidth (FSK only)
     *                          FSK : >= 2600 and <= 250000 Hz
     *                          LoRa: N/A ( set to 0 )
     * \param preambleLen  Sets the Preamble length
     *                          FSK : Number of bytes
     *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
     * \param symbTimeout  Sets the RxSingle timeout value
     *                          FSK : timeout in number of bytes
     *                          LoRa: timeout in symbols
     * \param fixLen       Fixed length packets [0: variable, 1: fixed]
     * \param payloadLen   Sets payload length when fixed length is used
     * \param crcOn        Enables/Disables the CRC [0: OFF, 1: ON]
     * \param freqHopOn    Enables disables the intra-packet frequency hopping
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: OFF, 1: ON]
     * \param hopPeriod    Number of symbols between each hop
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: Number of symbols
     * \param irqInverted   Inverts IQ signals (LoRa only)
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: not inverted, 1: inverted]
     * \param rxContinuous Sets the reception in continuous mode
     *                          [false: single mode, true: continuous mode]
     */
	void (*SetRxConfig)(     SX1262_ID id,
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
                              bool rxContinuous);
	/*!
     * \brief Sets the transmission parameters
     * \param id - SX1262 ID
     * \param modem        Radio modem to be used [0: FSK, 1: LoRa]
     * \param power        Sets the output power [dBm]
     * \param fdev         Sets the frequency deviation (FSK only)
     *                          FSK : [Hz]
     *                          LoRa: 0
     * \param bandwidth    Sets the bandwidth (LoRa only)
     *                          FSK : 0
     *                          LoRa: [0: 125 kHz, 1: 250 kHz,
     *                                 2: 500 kHz, 3: Reserved]
     * \param datarate     Sets the Datarate
     *                          FSK : 600..300000 bits/s
     *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
     *                                10: 1024, 11: 2048, 12: 4096  chips]
     * \param coderate     Sets the coding rate (LoRa only)
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     * \param preambleLen  Sets the preamble length
     *                          FSK : Number of bytes
     *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
     * \param fixLen       Fixed length packets [0: variable, 1: fixed]
     * \param crcOn        Enables disables the CRC [0: OFF, 1: ON]
     * \param freqHopOn    Enables disables the intra-packet frequency hopping
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: OFF, 1: ON]
     * \param hopPeriod    Number of symbols between each hop
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: Number of symbols
     * \param iqInverted   Inverts IQ signals (LoRa only)
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: not inverted, 1: inverted]
     * \param timeout      Transmission timeout [ms]
     */
	void (*SetTxConfig)(     SX1262_ID id,
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
                              uint32_t timeout);
	/*!
     * \brief Checks if the given RF frequency is supported by the hardware
     * \param id - SX1262 ID
     * \param frequency RF frequency to be checked
     * \retva isSupported [true: supported, false: unsupported]
     */
	bool (*CheckRfFrequency)(SX1262_ID id, uint32_t frequency);
	/*!
     * \brief Computes the packet time on air in ms for the given payload
     *
     * \remark Can only be called once SetRxConfig or SetTxConfig have been called
     * \param id - SX1262 ID
     * \param modem      Radio modem to be used [0: FSK, 1: LoRa]
     * \param pktLen     Packet payload length
     *
     * \retval airTime        Computed airTime (ms) for the given packet payload length
     */
	uint32_t (*TimeOnAir)(SX1262_ID id, RadioModems_t modem, uint8_t pktLen);
	/*!
     * \brief Sends the buffer of size. Prepares the packet to be sent and sets
     *        the radio in transmission
     * \param id - SX1262 ID
     * \param buffer     Buffer pointer
     * \param size       Buffer size
     */
	void (*Send)(SX1262_ID id, const uint8_t* const buffer, uint8_t size);
	/*!
     * \brief Sets the radio in sleep mode
     * \param id - SX1262 ID
     */
	void (*Sleep)(SX1262_ID id);
	/*!
     * \brief Sets the radio in standby mode
     * \param id - SX1262 ID
     */
	void (*Standby)(SX1262_ID id);
	/*!
     * \brief Sets the radio in reception mode for the given time
     * \param id - SX1262 ID
     * \param timeout Reception timeout [ms]
     *                     [0: continuous, others timeout]
     */
	void (*Rx)(SX1262_ID id, uint32_t timeout);
	/*!
     * \brief Set Channel Activity Detection parameters
     * \param id - SX1262 ID
     */
	void (*SetCadParams)(SX1262_ID id, uint8_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin, uint8_t cadExitMode, uint32_t cadTimeout);
	/*!
     * \brief Start a Channel Activity Detection
     * \param id - SX1262 ID
     */
	void (*StartCad)(SX1262_ID id);
	/*!
     * \brief Sets the radio in continuous wave transmission mode
     * \param id - SX1262 ID
     * \param freq       Channel RF frequency
     * \param power      Sets the output power [dBm]
     * \param time       Transmission mode timeout [s]
     */
	void (*SetTxContinuousWave)(SX1262_ID id, uint32_t freq, int8_t power, uint16_t time);
	/*!
     * \brief Reads the current RSSI value
     * \param id - SX1262 ID
     * \retval rssiValue Current RSSI value in [dBm]
     */
	int16_t (*Rssi)(SX1262_ID id, RadioModems_t modem);
	/*!
     * \brief Writes the radio register at the specified address
     * \param id - SX1262 ID
     * \param addr Register address
     * \param data New register value
     */
	void (*Write)(SX1262_ID id, uint16_t addr, uint8_t data);
	/*!
     * \brief Reads the radio register at the specified address
     * \param id - SX1262 ID
     * \param addr Register address
     * \retval data Register value
     */
	uint8_t (*Read)(SX1262_ID id, uint16_t addr);
	/*!
     * \brief Writes multiple radio registers starting at address
     * \param id - SX1262 ID
     * \param addr   First Radio register address
     * \param buffer Buffer containing the new register's values
     * \param size   Number of registers to be written
     */
	void (*WriteBuffer)(SX1262_ID id, uint16_t addr, const uint8_t* const buffer, uint8_t size);
	/*!
     * \brief Reads multiple radio registers starting at address
     * \param id - SX1262 ID
     * \param addr First Radio register address
     * \param buffer Buffer where to copy the registers data
     * \param size Number of registers to be read
     */
	void (*ReadBuffer)(SX1262_ID id, uint16_t addr, uint8_t* const buffer, uint8_t size);
	/*!
     * \brief Sets the maximum payload length.
     * \param id - SX1262 ID
     * \param modem      Radio modem to be used [0: FSK, 1: LoRa]
     * \param max        Maximum payload length in bytes
     */
	void (*SetMaxPayloadLength)(SX1262_ID id, RadioModems_t modem, uint8_t max);
	/*!
     * \brief Sets the network to public or private. Updates the sync byte.
     *
     * \remark Applies to LoRa modem only
     * \param id - SX1262 ID
     * \param enable if true, it enables a public network
     */
	void (*SetPublicNetwork)(SX1262_ID id, bool enable);
	/*!
     * \brief Gets the time required for the board plus radio to get out of sleep.[ms]
     * \param id - SX1262 ID
     * \retval time Radio plus board wakeup time in ms.
     */
	uint32_t (*GetWakeupTime)(SX1262_ID id);
	/*!
     * \brief Process radio irq in background task (nRF52 & ESP32)
     * \param id - SX1262 ID
     */
	void (*BgIrqProcess)(SX1262_ID id);
	/*!
     * \brief Process radio irq
     */
	void (*IrqProcess)(void);
	/*
      * \brief Process radio irq after CPU wakeup from deep sleep
     */
#if 0
	void (*IrqProcessAfterDeepSleep)(SX1262_ID id);
#endif
	/*
     * The next functions are available only on SX126x radios.
     */
	/*!
     * \brief Sets the radio in reception mode with Max LNA gain for the given time
     *
     * \remark Available on SX126x radios only.
     * \param id - SX1262 ID
     * \param timeout Reception timeout [ms]; [0: continuous, others timeout]
     */
	void (*RxBoosted)(SX1262_ID id, uint32_t timeout);
	/*!
     * \brief Sets the Rx duty cycle management parameters
     *
     * \remark Available on SX126x radios only.
     * \param id - SX1262 ID
     * \param rxTime        Structure describing reception timeout value
     * \param sleepTime     Structure describing sleep timeout value
     */
	void (*SetRxDutyCycle)(SX1262_ID id, uint32_t rxTime, uint32_t sleepTime);
} Radio_t;

/*!
 * \brief Radio driver
 *
 * \remark This variable is defined and initialized in the specific radio
 *         board implementation
 */
extern const Radio_t Radio;
#endif

#endif
