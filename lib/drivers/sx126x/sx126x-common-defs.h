/* NOTE:
 * This file includes common (LoRa/FSK) definitions
 * */
#ifndef SX126X_COMMON_DEFS_H
#define SX126X_COMMON_DEFS_H

#include "sx126x-fsk-defs.h"
#include "sx126x-lora-defs.h"

/*!
 * Radio complete Wake-up Time with TCXO stabilisation time
 */
#define RADIO_TCXO_SETUP_TIME               50      /* ms */
#define RADIO_WAKEUP_TIME                   3       /* ms */

/* SX1261/2 Data SheetDS.SX1261-2.W.APP 6.2.3.5 CRC */
/*!
 * \brief LFSR initial value to compute IBM type CRC
 */
#define CRC_IBM_SEED                        0xFFFF

/*!
 * \brief LFSR initial value to compute CCIT type CRC
 */
#define CRC_POLYNOMIAL_IBM                  0x8005

/*!
 * \brief Polynomial used to compute IBM CRC
 */
#define CRC_CCITT_SEED                      0x1D0F

/*!
 * \brief Polynomial used to compute CCIT CRC
 */
#define CRC_POLYNOMIAL_CCITT                0x1021

#define SX126X_LORA_SYNC_WORD_PUBLIC        0x3444
#define SX126X_LORA_SYNC_WORD_PRIVATE       0x1424

/* SX126X_CMD_SET_RX */
#define SX126X_RX_TIMEOUT_NONE                        0x000000    //  23    0     Rx timeout duration: no timeout (Rx single mode)
#define SX126X_RX_TIMEOUT_INF                         0xFFFFFF    //  23    0                          infinite (Rx continuous mode)

/* SX126X_CMD_STOP_TIMER_ON_PREAMBLE */
#define SX126X_STOP_ON_PREAMBLE_OFF                   0x00        //  7     0     stop timer on: sync word or header (default)
#define SX126X_STOP_ON_PREAMBLE_ON                    0x01        //  7     0                    preamble detection

/* SX126X_CMD_CALIBRATE_IMAGE */
#define SX126X_CAL_IMG_430_MHZ_1                      0x6B
#define SX126X_CAL_IMG_430_MHZ_2                      0x6F
#define SX126X_CAL_IMG_470_MHZ_1                      0x75
#define SX126X_CAL_IMG_470_MHZ_2                      0x81
#define SX126X_CAL_IMG_779_MHZ_1                      0xC1
#define SX126X_CAL_IMG_779_MHZ_2                      0xC5
#define SX126X_CAL_IMG_863_MHZ_1                      0xD7
#define SX126X_CAL_IMG_863_MHZ_2                      0xDB
#define SX126X_CAL_IMG_902_MHZ_1                      0xE1
#define SX126X_CAL_IMG_902_MHZ_2                      0xE9

//SX126X_CMD_SET_PA_CONFIG
#define SX126X_PA_CONFIG_HP_MAX                       0x07
#define SX126X_PA_CONFIG_SX1268                       0x01
#define SX126X_PA_CONFIG_PA_LUT                       0x01

//SX126X_CMD_SET_RX_TX_FALLBACK_MODE
#define SX126X_RX_TX_FALLBACK_MODE_FS                 0x40        //  7     0     after Rx/Tx go to: FS mode
#define SX126X_RX_TX_FALLBACK_MODE_STDBY_XOSC         0x30        //  7     0                        standby with crystal oscillator
#define SX126X_RX_TX_FALLBACK_MODE_STDBY_RC           0x20        //  7     0                        standby with RC oscillator (default)

/* SX126X_CMD_SET_DIO2_AS_RF_SWITCH_CTRL */
#define SX126X_DIO2_AS_IRQ                            0x00        //  7     0     DIO2 configuration: IRQ
#define SX126X_DIO2_AS_RF_SWITCH                      0x01        //  7     0                         RF switch control

//SX126X_CMD_GET_STATUS
#define SX126X_STATUS_MODE_STDBY_RC                   0b00100000  //  6     4     current chip mode: STDBY_RC
#define SX126X_STATUS_MODE_STDBY_XOSC                 0b00110000  //  6     4                        STDBY_XOSC
#define SX126X_STATUS_MODE_FS                         0b01000000  //  6     4                        FS
#define SX126X_STATUS_MODE_RX                         0b01010000  //  6     4                        RX
#define SX126X_STATUS_MODE_TX                         0b01100000  //  6     4                        TX
#define SX126X_STATUS_DATA_AVAILABLE                  0b00000100  //  3     1     command status: packet received and data can be retrieved
#define SX126X_STATUS_CMD_TIMEOUT                     0b00000110  //  3     1                     SPI command timed out
#define SX126X_STATUS_CMD_INVALID                     0b00001000  //  3     1                     invalid SPI command
#define SX126X_STATUS_CMD_FAILED                      0b00001010  //  3     1                     SPI command failed to execute
#define SX126X_STATUS_TX_DONE                         0b00001100  //  3     1                     packet transmission done


//SX126X_CMD_GET_DEVICE_ERRORS
#define SX126X_PA_RAMP_ERR                           0b100000000  //  8     8     device errors: PA ramping failed
#define SX126X_PLL_LOCK_ERR                          0b001000000  //  6     6                    PLL failed to lock
#define SX126X_XOSC_START_ERR                        0b000100000  //  5     5                    crystal oscillator failed to start
#define SX126X_IMG_CALIB_ERR                         0b000010000  //  4     4                    image calibration failed
#define SX126X_ADC_CALIB_ERR                         0b000001000  //  3     3                    ADC calibration failed
#define SX126X_PLL_CALIB_ERR                         0b000000100  //  2     2                    PLL calibration failed
#define SX126X_RC13M_CALIB_ERR                       0b000000010  //  1     1                    RC13M calibration failed
#define SX126X_RC64K_CALIB_ERR                       0b000000001  //  0     0                    RC64K calibration failed

#define SX126X_TXMODE_ASYNC                           0x01
#define SX126X_TXMODE_SYNC                            0x02
#define SX126X_TXMODE_BACK2RX                         0x04

/* TCXO voltage */
typedef enum
{
    TCXO_CTRL_1_6V = 0x00,
    TCXO_CTRL_1_7V = 0x01,
    TCXO_CTRL_1_8V = 0x02,
    TCXO_CTRL_2_2V = 0x03,
    TCXO_CTRL_2_4V = 0x04,
    TCXO_CTRL_2_7V = 0x05,
    TCXO_CTRL_3_0V = 0x06,
    TCXO_CTRL_3_3V = 0x07,
} TCXO_VOLTAGE;

/* Calibration configuration */
typedef union
{
    struct
    {
        uint8_t RC64KEnable : 1;    //!< Calibrate RC64K clock
        uint8_t RC13MEnable : 1;    //!< Calibrate RC13M clock
        uint8_t PLLEnable : 1;      //!< Calibrate PLL
        uint8_t ADCPulseEnable : 1; //!< Calibrate ADC Pulse
        uint8_t ADCBulkNEnable : 1; //!< Calibrate ADC bulkN
        uint8_t ADCBulkPEnable : 1; //!< Calibrate ADC bulkP
        uint8_t ImgEnable : 1;
        uint8_t : 1;
    } Fields;
    uint8_t Value;
} CalibrationParams_t;

/* Operating mode the radio is actually running */
typedef enum
{
    MODE_SLEEP = 0x00, //! The radio is in sleep mode
    MODE_STDBY_RC,     //! The radio is in standby mode with RC oscillator
    MODE_STDBY_XOSC,   //! The radio is in standby mode with XOSC oscillator
    MODE_FS,           //! The radio is in frequency synthesis mode
    MODE_TX,           //! The radio is in transmit mode
    MODE_RX,           //! The radio is in receive mode
    MODE_RX_DC,        //! The radio is in receive duty cycle mode
    MODE_CAD           //! The radio is in channel activity detection mode
} OPERATION_MODES;

/* Supported packet types */
typedef enum
{
    PACKET_TYPE_GFSK = 0x00,
    PACKET_TYPE_LORA = 0x01,
    PACKET_TYPE_NONE = 0x0F,
} PACKET_TYPES;

typedef union
{
    struct
    {
        uint8_t WakeUpRTC : 1;
        uint8_t Reset : 1;
        uint8_t WarmStart : 1;
        uint8_t Reserved : 5;
    } Fields;
    uint8_t Value;
} SleepParams_t;

typedef enum
{
    STDBY_RC = 0x00,
    STDBY_XOSC = 0x01,
} STANDBY_MODES;

typedef enum
{
    RAMP_10_US = 0x00,
    RAMP_20_US = 0x01,
    RAMP_40_US = 0x02,
    RAMP_80_US = 0x03,
    RAMP_200_US = 0x04,
    RAMP_800_US = 0x05,
    RAMP_1700_US = 0x06,
    RAMP_3400_US = 0x07,
} RAMP_TIME;

typedef enum
{
    USE_LDO = 0x00,
    USE_DCDC = 0x01,
} REAGULATOR_MODES;

/*!
 * \brief The type describing the modulation parameters for every packet types
 */
typedef struct
{
    PACKET_TYPES packetType; //!< Packet to which the modulation parameters are referring to.
    struct
    {
        struct
        {
            uint32_t bitRate;
            uint32_t fdev;
            PULSE_SHAPINGS modulationShaping;
            uint8_t bandwidth;
        } Gfsk;
        struct
        {
            SPREAD_FACTORS spreadingFactor;
            BANDWIDTHS bandwidth;
            CODING_RATES codingRate;
            uint8_t lowDatarateOptimize
        } LoRa;
    } Params; //!< Holds the modulation parameters structure
} ModulationParams_t;

/*!
 * \brief The type describing the packet parameters for every packet types
 */
typedef struct
{
    PACKET_TYPES packetType; //!< Packet to which the packet parameters are referring to.
    struct
    {
        /*!
         * \brief Holds the GFSK packet parameters
         */
        struct
        {
            uint16_t preambleLength;                    //!< The preamble Tx length for GFSK packet type in bit
            PREAMBLE_DETECTOR_LEN PreambleMinDetect; //!< The preamble Rx length minimal for GFSK packet type
            uint8_t syncWordLength;                     //!< The synchronization word length for GFSK packet type
            ADDR_COMP addrComp;                //!< Activated SyncWord correlators
            PACKET_LEN headerType;        //!< If the header is explicit, it will be transmitted in the GFSK packet. If the header is implicit, it will not be transmitted
            uint8_t payloadLength;                      //!< Size of the payload in the GFSK packet
            CRC_TYPES crcType;                  //!< Size of the CRC block in the GFSK packet
            WHITENING whitening;
        } Gfsk;
        /*!
         * \brief Holds the LoRa packet parameters
         */
        struct
        {
            uint16_t preambleLength;    //!< The preamble length is the number of LoRa symbols in the preamble
            HEADER_TYPES headerType;    //!< If the header is explicit, it will be transmitted in the LoRa packet. If the header is implicit, it will not be transmitted
            uint8_t payloadLength;      //!< Size of the payload in the LoRa packet
            CRC_MODES crcMode;          //!< Size of CRC block in LoRa packet
            IQ_MODES invertIQ;          //!< Allows to swap IQ for LoRa packet
        } LoRa;
    } Params; //!< Holds the packet parameters structure
} PacketParams_t;

/*!
 * \brief Represents the packet status for every packet type
 */
typedef struct
{
    PACKET_TYPES packetType; //!< Packet to which the packet status are referring to.
    struct
    {
        struct
        {
            uint8_t rxStatus;
            int8_t rssiAvg;  //!< The averaged RSSI
            int8_t rssiSync; //!< The RSSI measured on last packet
            uint32_t freqError;
        } Gfsk;
        struct
        {
            int8_t rssiPkt; //!< The RSSI of the last packet
            int8_t snrPkt;  //!< The SNR of the last packet
            int8_t signalRssiPkt;
            uint32_t freqError;
        } LoRa;
    } Params;
} PacketStatus_t;

/*!
 * \brief Structure describing sx126x status
 */
typedef union RadioStatus_u
{
    uint8_t value;
    struct
    {                          //bit order is lsb -> msb
        uint8_t reserved : 1;  //!< Reserved
        uint8_t cmdStatus : 3; //!< Command status
        uint8_t chipMode : 3;  //!< Chip mode
        uint8_t cpuBusy : 1;   //!< Flag for CPU radio busy
    } Fields;
} SX126x_Status_t;

/*!
 * \brief Represents the possible sx126x system error states
 */
typedef union
{
    struct
    {
        uint8_t rc64kCalib : 1; //!< RC 64kHz oscillator calibration failed
        uint8_t rc13mCalib : 1; //!< RC 13MHz oscillator calibration failed
        uint8_t pllCalib : 1;   //!< PLL calibration failed
        uint8_t adcCalib : 1;   //!< ADC calibration failed
        uint8_t imgCalib : 1;   //!< Image calibration failed
        uint8_t xoscStart : 1;  //!< XOSC oscillator failed to start
        uint8_t pllLock : 1;    //!< PLL lock failed
        uint8_t buckStart : 1;  //!< Buck converter failed to start
        uint8_t paRamp : 1;     //!< PA ramp failed
        uint8_t : 7;            //!< Reserved
    } Fields;
    uint16_t Value;
} SX126x_Error_t;

/*!
 * sx126x global parameters
 */
typedef struct
{
    PacketParams_t packetParams;
    PacketStatus_t packetStatus;
    ModulationParams_t modulationParams;
} SX126x_Params_t;

/*!
 * \brief Represents the interruption masks available for the radio
 *
 * \remark Note that not all these interruptions are available for all packet types
 */
typedef enum
{
    IRQ_NONE = 0x0000,
    IRQ_TX_DONE = 0x0001,
    IRQ_RX_DONE = 0x0002,
    IRQ_PREAMBLE_DETECTED = 0x0004,
    IRQ_SYNCWORD_VALID = 0x0008,
    IRQ_HEADER_VALID = 0x0010,
    IRQ_HEADER_ERROR = 0x0020,
    IRQ_CRC_ERROR = 0x0040,
    IRQ_CAD_DONE = 0x0080,
    IRQ_CAD_ACTIVITY_DETECTED = 0x0100,
    IRQ_RX_TX_TIMEOUT = 0x0200,
    IRQ_TX_ALL = 0x0201,
    IRQ_RX_ALL = 0x027E,
    IRQ_ALL = 0xFFFF,
} IRQ_MASKS;

/* DIO1 IRQ handler */
typedef void(DioIrqHandler)(void);

#endif /* SX126X_COMMON_DEFS_H */
