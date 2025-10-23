#ifndef SX126X_DEFINITIONS_H
#define SX126X_DEFINITIONS_H

#include <stdint.h>
#include <stdbool.h>

/* SX126x supported IDs */
typedef enum
{
    SX1262_ID_TX = 0,
    SX1262_ID_RX,
    SX1262_ID_NUMBER
} SX1262_ID;

/* SX126x commands */
typedef enum RadioCommands_e
{
    RADIO_GET_STATUS = 0xC0,
    RADIO_WRITE_REGISTER = 0x0D,
    RADIO_READ_REGISTER = 0x1D,
    RADIO_WRITE_BUFFER = 0x0E,
    RADIO_READ_BUFFER = 0x1E,
    RADIO_SET_SLEEP = 0x84,
    RADIO_SET_STANDBY = 0x80,
    RADIO_SET_FS = 0xC1,
    RADIO_SET_TX = 0x83,
    RADIO_SET_RX = 0x82,
    RADIO_SET_RXDUTYCYCLE = 0x94,
    RADIO_SET_CAD = 0xC5,
    RADIO_SET_TXCONTINUOUSWAVE = 0xD1,
    RADIO_SET_TXCONTINUOUSPREAMBLE = 0xD2,
    RADIO_SET_PACKETTYPE = 0x8A,
    RADIO_GET_PACKETTYPE = 0x11,
    RADIO_SET_RFFREQUENCY = 0x86,
    RADIO_SET_TXPARAMS = 0x8E,
    RADIO_SET_PACONFIG = 0x95,
    RADIO_SET_CADPARAMS = 0x88,
    RADIO_SET_BUFFERBASEADDRESS = 0x8F,
    RADIO_SET_MODULATIONPARAMS = 0x8B,
    RADIO_SET_PACKETPARAMS = 0x8C,
    RADIO_GET_RXBUFFERSTATUS = 0x13,
    RADIO_GET_PACKETSTATUS = 0x14,
    RADIO_GET_RSSIINST = 0x15,
    RADIO_GET_STATS = 0x10,
    RADIO_RESET_STATS = 0x00,
    RADIO_CFG_DIOIRQ = 0x08,
    RADIO_GET_IRQSTATUS = 0x12,
    RADIO_CLR_IRQSTATUS = 0x02,
    RADIO_CALIBRATE = 0x89,
    RADIO_CALIBRATEIMAGE = 0x98,
    RADIO_SET_REGULATORMODE = 0x96,
    RADIO_GET_ERROR = 0x17,
    RADIO_CLR_ERROR = 0x07,
    RADIO_SET_TCXOMODE = 0x97,
    RADIO_SET_TXFALLBACKMODE = 0x93,
    RADIO_SET_RFSWITCHMODE = 0x9D,
    RADIO_SET_STOPRXTIMERONPREAMBLE = 0x9F,
    RADIO_SET_LORASYMBTIMEOUT = 0xA0,
} RadioCommands_t;

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
} RadioTcxoCtrlVoltage_t;

/* SX126x hardware config */
typedef struct
{
    uint8_t CHIP_TYPE;
    /* SPI gpio */
    uint8_t PIN_LORA_MISO;
    uint8_t PIN_LORA_MOSI;
    uint8_t PIN_LORA_SCLK;

    /* board specific gpio */
    uint8_t PIN_LORA_RESET;
    uint8_t PIN_LORA_NSS;
    uint8_t PIN_LORA_DIO;
    uint8_t PIN_LORA_BUSY;
    
    /* board specifc options */
    uint8_t RADIO_TXEN;
    uint8_t RADIO_RXEN;
    bool USE_DIO2_ANT_SWITCH;
    bool USE_DIO3_TCXO;
    bool USE_DIO3_ANT_SWITCH;
    bool USE_LDO;
    bool USE_RXEN_ANT_PWR;
    RadioTcxoCtrlVoltage_t TCXO_CTRL_VOLTAGE;
} BoardConfig_t;

/* Represents a calibration configuration */
typedef union
{
    struct
    {
        uint8_t RC64KEnable : 1;	//!< Calibrate RC64K clock
        uint8_t RC13MEnable : 1;	//!< Calibrate RC13M clock
        uint8_t PLLEnable : 1;		//!< Calibrate PLL
        uint8_t ADCPulseEnable : 1; //!< Calibrate ADC Pulse
        uint8_t ADCBulkNEnable : 1; //!< Calibrate ADC bulkN
        uint8_t ADCBulkPEnable : 1; //!< Calibrate ADC bulkP
        uint8_t ImgEnable : 1;
        uint8_t : 1;
    } Fields;
    uint8_t Value;
} CalibrationParams_t;

/* Represents the operating mode the radio is actually running */
typedef enum
{
    MODE_SLEEP = 0x00, //! The radio is in sleep mode
    MODE_STDBY_RC,	   //! The radio is in standby mode with RC oscillator
    MODE_STDBY_XOSC,   //! The radio is in standby mode with XOSC oscillator
    MODE_FS,		   //! The radio is in frequency synthesis mode
    MODE_TX,		   //! The radio is in transmit mode
    MODE_RX,		   //! The radio is in receive mode
    MODE_RX_DC,		   //! The radio is in receive duty cycle mode
    MODE_CAD		   //! The radio is in channel activity detection mode
} RadioOperatingModes_t;

typedef enum
{
    PACKET_TYPE_GFSK = 0x00,
    PACKET_TYPE_LORA = 0x01,
    PACKET_TYPE_NONE = 0x0F,
} RadioPacketTypes_t;

typedef union
{
    struct
    {
        uint8_t WakeUpRTC : 1; //!< Get out of sleep mode if wakeup signal received from RTC
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
} RadioStandbyModes_t;

typedef enum
{
    RADIO_RAMP_10_US = 0x00,
    RADIO_RAMP_20_US = 0x01,
    RADIO_RAMP_40_US = 0x02,
    RADIO_RAMP_80_US = 0x03,
    RADIO_RAMP_200_US = 0x04,
    RADIO_RAMP_800_US = 0x05,
    RADIO_RAMP_1700_US = 0x06,
    RADIO_RAMP_3400_US = 0x07,
} RadioRampTimes_t;

typedef enum
{
    LORA_CAD_01_SYMBOL = 0x00,
    LORA_CAD_02_SYMBOL = 0x01,
    LORA_CAD_04_SYMBOL = 0x02,
    LORA_CAD_08_SYMBOL = 0x03,
    LORA_CAD_16_SYMBOL = 0x04,
} RadioLoRaCadSymbols_t;

/*!
 * \brief Represents the Channel Activity Detection actions after the CAD operation is finished
 */
typedef enum
{
    LORA_CAD_ONLY = 0x00,
    LORA_CAD_RX = 0x01,
    LORA_CAD_LBT = 0x10,
} RadioCadExitModes_t;

/*!
 * \brief Represents the preamble length used to detect the packet on Rx side
 */
typedef enum
{
    RADIO_PREAMBLE_DETECTOR_OFF = 0x00,		//!< Preamble detection length off
    RADIO_PREAMBLE_DETECTOR_08_BITS = 0x04, //!< Preamble detection length 8 bits
    RADIO_PREAMBLE_DETECTOR_16_BITS = 0x05, //!< Preamble detection length 16 bits
    RADIO_PREAMBLE_DETECTOR_24_BITS = 0x06, //!< Preamble detection length 24 bits
    RADIO_PREAMBLE_DETECTOR_32_BITS = 0x07, //!< Preamble detection length 32 bit
} RadioPreambleDetection_t;

/*!
 * \brief Represents the modulation shaping parameter
 */
typedef enum
{
    MOD_SHAPING_OFF = 0x00,
    MOD_SHAPING_G_BT_03 = 0x08,
    MOD_SHAPING_G_BT_05 = 0x09,
    MOD_SHAPING_G_BT_07 = 0x0A,
    MOD_SHAPING_G_BT_1 = 0x0B,
} RadioModShapings_t;

/*!
 * \brief Represents the possible spreading factor values in LoRa packet types
 */
typedef enum
{
    LORA_SF5 = 0x05,
    LORA_SF6 = 0x06,
    LORA_SF7 = 0x07,
    LORA_SF8 = 0x08,
    LORA_SF9 = 0x09,
    LORA_SF10 = 0x0A,
    LORA_SF11 = 0x0B,
    LORA_SF12 = 0x0C,
} RadioLoRaSpreadingFactors_t;

/*!
 * \brief Represents the bandwidth values for LoRa packet type
 */
typedef enum
{
    LORA_BW_500 = 6,
    LORA_BW_250 = 5,
    LORA_BW_125 = 4,
    LORA_BW_062 = 3,
    LORA_BW_041 = 10,
    LORA_BW_031 = 2,
    LORA_BW_020 = 9,
    LORA_BW_015 = 1,
    LORA_BW_010 = 8,
    LORA_BW_007 = 0,
} RadioLoRaBandwidths_t;

/*!
 * \brief Represents the coding rate values for LoRa packet type
 */
typedef enum
{
    LORA_CR_4_5 = 0x01,
    LORA_CR_4_6 = 0x02,
    LORA_CR_4_7 = 0x03,
    LORA_CR_4_8 = 0x04,
} RadioLoRaCodingRates_t;

/*!
 * \brief The type describing the modulation parameters for every packet types
 */
typedef struct
{
    RadioPacketTypes_t PacketType; //!< Packet to which the modulation parameters are referring to.
    struct
    {
        struct
        {
            uint32_t BitRate;
            uint32_t Fdev;
            RadioModShapings_t ModulationShaping;
            uint8_t Bandwidth;
        } Gfsk;
        struct
        {
            RadioLoRaSpreadingFactors_t SpreadingFactor; //!< Spreading Factor for the LoRa modulation
            RadioLoRaBandwidths_t Bandwidth;			 //!< Bandwidth for the LoRa modulation
            RadioLoRaCodingRates_t CodingRate;			 //!< Coding rate for the LoRa modulation
            uint8_t LowDatarateOptimize;				 //!< Indicates if the modem uses the low datarate optimization
        } LoRa;
    } Params; //!< Holds the modulation parameters structure
} ModulationParams_t;

/*!
 * \brief Represents the possible combinations of SyncWord correlators activated
 */
typedef enum
{
    RADIO_ADDRESSCOMP_FILT_OFF = 0x00, //!< No correlator turned on, i.e. do not search for SyncWord
    RADIO_ADDRESSCOMP_FILT_NODE = 0x01,
    RADIO_ADDRESSCOMP_FILT_NODE_BROAD = 0x02,
} RadioAddressComp_t;

/*!
 *  \brief Radio GFSK packet length mode
 */
typedef enum
{
    RADIO_PACKET_FIXED_LENGTH = 0x00,	 //!< The packet is known on both sides, no header included in the packet
    RADIO_PACKET_VARIABLE_LENGTH = 0x01, //!< The packet is on variable size, header included
} RadioPacketLengthModes_t;

/*!
 * \brief Radio whitening mode activated or deactivated
 */
typedef enum
{
    RADIO_DC_FREE_OFF = 0x00,
    RADIO_DC_FREEWHITENING = 0x01,
} RadioDcFree_t;

/*!
 * \brief Holds the Radio lengths mode for the LoRa packet type
 */
typedef enum
{
    LORA_PACKET_VARIABLE_LENGTH = 0x00, //!< The packet is on variable size, header included
    LORA_PACKET_FIXED_LENGTH = 0x01,	//!< The packet is known on both sides, no header included in the packet
    LORA_PACKET_EXPLICIT = LORA_PACKET_VARIABLE_LENGTH,
    LORA_PACKET_IMPLICIT = LORA_PACKET_FIXED_LENGTH,
} RadioLoRaPacketLengthsMode_t;

/*!
 * \brief Represents the CRC mode for LoRa packet type
 */
typedef enum
{
    LORA_CRC_ON = 0x01,	 //!< CRC activated
    LORA_CRC_OFF = 0x00, //!< CRC not used
} RadioLoRaCrcModes_t;

/*!
 * \brief Represents the IQ mode for LoRa packet type
 */
typedef enum
{
    LORA_IQ_NORMAL = 0x00,
    LORA_IQ_INVERTED = 0x01,
} RadioLoRaIQModes_t;

typedef enum
{
    RADIO_CRC_OFF = 0x01, //!< No CRC in use
    RADIO_CRC_1_BYTES = 0x00,
    RADIO_CRC_2_BYTES = 0x02,
    RADIO_CRC_1_BYTES_INV = 0x04,
    RADIO_CRC_2_BYTES_INV = 0x06,
    RADIO_CRC_2_BYTES_IBM = 0xF1,
    RADIO_CRC_2_BYTES_CCIT = 0xF2,
} RadioCrcTypes_t;

/*!
 * \brief The type describing the packet parameters for every packet types
 */
typedef struct
{
    RadioPacketTypes_t PacketType; //!< Packet to which the packet parameters are referring to.
    struct
    {
        /*!
         * \brief Holds the GFSK packet parameters
         */
        struct
        {
            uint16_t PreambleLength;					//!< The preamble Tx length for GFSK packet type in bit
            RadioPreambleDetection_t PreambleMinDetect; //!< The preamble Rx length minimal for GFSK packet type
            uint8_t SyncWordLength;						//!< The synchronization word length for GFSK packet type
            RadioAddressComp_t AddrComp;				//!< Activated SyncWord correlators
            RadioPacketLengthModes_t HeaderType;		//!< If the header is explicit, it will be transmitted in the GFSK packet. If the header is implicit, it will not be transmitted
            uint8_t PayloadLength;						//!< Size of the payload in the GFSK packet
            RadioCrcTypes_t CrcLength;					//!< Size of the CRC block in the GFSK packet
            RadioDcFree_t DcFree;
        } Gfsk;
        /*!
         * \brief Holds the LoRa packet parameters
         */
        struct
        {
            uint16_t PreambleLength;				 //!< The preamble length is the number of LoRa symbols in the preamble
            RadioLoRaPacketLengthsMode_t HeaderType; //!< If the header is explicit, it will be transmitted in the LoRa packet. If the header is implicit, it will not be transmitted
            uint8_t PayloadLength;					 //!< Size of the payload in the LoRa packet
            RadioLoRaCrcModes_t CrcMode;			 //!< Size of CRC block in LoRa packet
            RadioLoRaIQModes_t InvertIQ;			 //!< Allows to swap IQ for LoRa packet
        } LoRa;
    } Params; //!< Holds the packet parameters structure
} PacketParams_t;

/*!
 * \brief Represents the packet status for every packet type
 */
typedef struct
{
    RadioPacketTypes_t packetType; //!< Packet to which the packet status are referring to.
    struct
    {
        struct
        {
            uint8_t RxStatus;
            int8_t RssiAvg;	 //!< The averaged RSSI
            int8_t RssiSync; //!< The RSSI measured on last packet
            uint32_t FreqError;
        } Gfsk;
        struct
        {
            int8_t RssiPkt; //!< The RSSI of the last packet
            int8_t SnrPkt;	//!< The SNR of the last packet
            int8_t SignalRssiPkt;
            uint32_t FreqError;
        } LoRa;
    } Params;
} PacketStatus_t;

/*!
 * \brief Structure describing the radio status
 */
typedef union RadioStatus_u
{
    uint8_t Value;
    struct
    {						   //bit order is lsb -> msb
        uint8_t Reserved : 1;  //!< Reserved
        uint8_t CmdStatus : 3; //!< Command status
        uint8_t ChipMode : 3;  //!< Chip mode
        uint8_t CpuBusy : 1;   //!< Flag for CPU radio busy
    } Fields;
} RadioStatus_t;

/*!
 * \brief Represents the possible radio system error states
 */
typedef union
{
    struct
    {
        uint8_t Rc64kCalib : 1; //!< RC 64kHz oscillator calibration failed
        uint8_t Rc13mCalib : 1; //!< RC 13MHz oscillator calibration failed
        uint8_t PllCalib : 1;	//!< PLL calibration failed
        uint8_t AdcCalib : 1;	//!< ADC calibration failed
        uint8_t ImgCalib : 1;	//!< Image calibration failed
        uint8_t XoscStart : 1;	//!< XOSC oscillator failed to start
        uint8_t PllLock : 1;	//!< PLL lock failed
        uint8_t BuckStart : 1;	//!< Buck converter failed to start
        uint8_t PaRamp : 1;		//!< PA ramp failed
        uint8_t : 7;			//!< Reserved
    } Fields;
    uint16_t Value;
} RadioError_t;

/*!
 * Radio global parameters
 */
typedef struct SX126x_s
{
	PacketParams_t PacketParams;
	PacketStatus_t PacketStatus;
	ModulationParams_t ModulationParams;
} sx126xParams_t;

typedef enum
{
    USE_LDO = 0x00, // default
    USE_DCDC = 0x01,
} RadioRegulatorMode_t;

/*!
 * \brief Structure describing the error codes for callback functions
 */
typedef enum
{
	IRQ_HEADER_ERROR_CODE = 0x01,
	IRQ_SYNCWORD_ERROR_CODE = 0x02,
	IRQ_CRC_ERROR_CODE = 0x04,
} IrqErrorCode_t;

/* NOTE: not used */
#if 0
/*!
 * \brief The radio callbacks structure
 * Holds function pointers to be called on radio interrupts
 */
typedef struct
{
	void (*txDone)(void);					 //!< Pointer to a function run on successful transmission
	void (*rxDone)(void);					 //!< Pointer to a function run on successful reception
	void (*rxPreambleDetect)(void);			 //!< Pointer to a function run on successful Preamble detection
	void (*rxSyncWordDone)(void);			 //!< Pointer to a function run on successful SyncWord reception
	void (*rxHeaderDone)(bool isOk);		 //!< Pointer to a function run on successful Header reception
	void (*txTimeout)(void);				 //!< Pointer to a function run on transmission timeout
	void (*rxTimeout)(void);				 //!< Pointer to a function run on reception timeout
	void (*rxError)(IrqErrorCode_t errCode); //!< Pointer to a function run on reception error
	void (*cadDone)(bool cadFlag);			 //!< Pointer to a function run on channel activity detected
} SX126xCallbacks_t;
#endif

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

/*!
 * \brief Represents the interruption masks available for the radio
 *
 * \remark Note that not all these interruptions are available for all packet types
 */
typedef enum
{
	IRQ_RADIO_NONE = 0x0000,
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
	IRQ_RADIO_ALL = 0xFFFF,
} RadioIrqMasks_t;

/* SX126X_CMD_SET_DIO2_AS_RF_SWITCH_CTRL */
#define SX126X_DIO2_AS_IRQ                            0x00        //  7     0     DIO2 configuration: IRQ
#define SX126X_DIO2_AS_RF_SWITCH                      0x01        //  7     0                         RF switch control

/* SX126X_CMD_SET_MODULATION_PARAMS */
#define SX126X_GFSK_FILTER_NONE                       0x00        //  7     0     GFSK filter: none
#define SX126X_GFSK_FILTER_GAUSS_0_3                  0x08        //  7     0                  Gaussian, BT = 0.3
#define SX126X_GFSK_FILTER_GAUSS_0_5                  0x09        //  7     0                  Gaussian, BT = 0.5
#define SX126X_GFSK_FILTER_GAUSS_0_7                  0x0A        //  7     0                  Gaussian, BT = 0.7
#define SX126X_GFSK_FILTER_GAUSS_1                    0x0B        //  7     0                  Gaussian, BT = 1
#define SX126X_GFSK_RX_BW_4_8                         0x1F        //  7     0     GFSK Rx bandwidth: 4.8 kHz
#define SX126X_GFSK_RX_BW_5_8                         0x17        //  7     0                        5.8 kHz
#define SX126X_GFSK_RX_BW_7_3                         0x0F        //  7     0                        7.3 kHz
#define SX126X_GFSK_RX_BW_9_7                         0x1E        //  7     0                        9.7 kHz
#define SX126X_GFSK_RX_BW_11_7                        0x16        //  7     0                        11.7 kHz
#define SX126X_GFSK_RX_BW_14_6                        0x0E        //  7     0                        14.6 kHz
#define SX126X_GFSK_RX_BW_19_5                        0x1D        //  7     0                        19.5 kHz
#define SX126X_GFSK_RX_BW_23_4                        0x15        //  7     0                        23.4 kHz
#define SX126X_GFSK_RX_BW_29_3                        0x0D        //  7     0                        29.3 kHz
#define SX126X_GFSK_RX_BW_39_0                        0x1C        //  7     0                        39.0 kHz
#define SX126X_GFSK_RX_BW_46_9                        0x14        //  7     0                        46.9 kHz
#define SX126X_GFSK_RX_BW_58_6                        0x0C        //  7     0                        58.6 kHz
#define SX126X_GFSK_RX_BW_78_2                        0x1B        //  7     0                        78.2 kHz
#define SX126X_GFSK_RX_BW_93_8                        0x13        //  7     0                        93.8 kHz
#define SX126X_GFSK_RX_BW_117_3                       0x0B        //  7     0                        117.3 kHz
#define SX126X_GFSK_RX_BW_156_2                       0x1A        //  7     0                        156.2 kHz
#define SX126X_GFSK_RX_BW_187_2                       0x12        //  7     0                        187.2 kHz
#define SX126X_GFSK_RX_BW_234_3                       0x0A        //  7     0                        234.3 kHz
#define SX126X_GFSK_RX_BW_312_0                       0x19        //  7     0                        312.0 kHz
#define SX126X_GFSK_RX_BW_373_6                       0x11        //  7     0                        373.6 kHz
#define SX126X_GFSK_RX_BW_467_0                       0x09        //  7     0                        467.0 kHz
#define SX126X_LORA_BW_7_8                            0x00        //  7     0     LoRa bandwidth: 7.8 kHz
#define SX126X_LORA_BW_10_4                           0x08        //  7     0                     10.4 kHz
#define SX126X_LORA_BW_15_6                           0x01        //  7     0                     15.6 kHz
#define SX126X_LORA_BW_20_8                           0x09        //  7     0                     20.8 kHz
#define SX126X_LORA_BW_31_25                          0x02        //  7     0                     31.25 kHz
#define SX126X_LORA_BW_41_7                           0x0A        //  7     0                     41.7 kHz
#define SX126X_LORA_BW_62_5                           0x03        //  7     0                     62.5 kHz
#define SX126X_LORA_BW_125_0                          0x04        //  7     0                     125.0 kHz
#define SX126X_LORA_BW_250_0                          0x05        //  7     0                     250.0 kHz
#define SX126X_LORA_BW_500_0                          0x06        //  7     0                     500.0 kHz
#define SX126X_LORA_CR_4_5                            0x01        //  7     0     LoRa coding rate: 4/5
#define SX126X_LORA_CR_4_6                            0x02        //  7     0                       4/6
#define SX126X_LORA_CR_4_7                            0x03        //  7     0                       4/7
#define SX126X_LORA_CR_4_8                            0x04        //  7     0                       4/8
#define SX126X_LORA_LOW_DATA_RATE_OPTIMIZE_OFF        0x00        //  7     0     LoRa low data rate optimization: disabled
#define SX126X_LORA_LOW_DATA_RATE_OPTIMIZE_ON         0x01        //  7     0                                      enabled

#define SX126X_GFSK_ADDRESS_FILT_OFF                  0x00        //  7     0     GFSK address filtering: disabled
#define SX126X_GFSK_ADDRESS_FILT_NODE                 0x01        //  7     0                             node only
#define SX126X_GFSK_ADDRESS_FILT_NODE_BROADCAST       0x02        //  7     0                             node and broadcast
#define SX126X_GFSK_PACKET_FIXED                      0x00        //  7     0     GFSK packet type: fixed (payload length known in advance to both sides)
#define SX126X_GFSK_PACKET_VARIABLE                   0x01        //  7     0                       variable (payload length added to packet)
#define SX126X_GFSK_CRC_OFF                           0x01        //  7     0     GFSK packet CRC: disabled
#define SX126X_GFSK_CRC_1_BYTE                        0x00        //  7     0                      1 byte
#define SX126X_GFSK_CRC_2_BYTE                        0x02        //  7     0                      2 byte
#define SX126X_GFSK_CRC_1_BYTE_INV                    0x04        //  7     0                      1 byte, inverted
#define SX126X_GFSK_CRC_2_BYTE_INV                    0x06        //  7     0                      2 byte, inverted
#define SX126X_GFSK_WHITENING_OFF                     0x00        //  7     0     GFSK data whitening: disabled
#define SX126X_GFSK_WHITENING_ON                      0x01        //  7     0                          enabled
#define SX126X_LORA_HEADER_EXPLICIT                   0x00        //  7     0     LoRa header mode: explicit
#define SX126X_LORA_HEADER_IMPLICIT                   0x01        //  7     0                       implicit
#define SX126X_LORA_CRC_OFF                           0x00        //  7     0     LoRa CRC mode: disabled
#define SX126X_LORA_CRC_ON                            0x01        //  7     0                    enabled
#define SX126X_LORA_IQ_STANDARD                       0x00        //  7     0     LoRa IQ setup: standard
#define SX126X_LORA_IQ_INVERTED                       0x01        //  7     0                    inverted

//SX126X_CMD_SET_CAD_PARAMS
#define SX126X_CAD_ON_1_SYMB                          0x00        //  7     0     number of symbols used for CAD: 1
#define SX126X_CAD_ON_2_SYMB                          0x01        //  7     0                                     2
#define SX126X_CAD_ON_4_SYMB                          0x02        //  7     0                                     4
#define SX126X_CAD_ON_8_SYMB                          0x03        //  7     0                                     8
#define SX126X_CAD_ON_16_SYMB                         0x04        //  7     0                                     16
#define SX126X_CAD_GOTO_STDBY                         0x00        //  7     0     after CAD is done, always go to STDBY_RC mode
#define SX126X_CAD_GOTO_RX                            0x01        //  7     0     after CAD is done, go to Rx mode if activity is detected

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

//SX126X_CMD_GET_PACKET_STATUS
#define SX126X_GFSK_RX_STATUS_PREAMBLE_ERR            0b10000000  //  7     7     GFSK Rx status: preamble error
#define SX126X_GFSK_RX_STATUS_SYNC_ERR                0b01000000  //  6     6                     sync word error
#define SX126X_GFSK_RX_STATUS_ADRS_ERR                0b00100000  //  5     5                     address error
#define SX126X_GFSK_RX_STATUS_CRC_ERR                 0b00010000  //  4     4                     CRC error
#define SX126X_GFSK_RX_STATUS_LENGTH_ERR              0b00001000  //  3     3                     length error
#define SX126X_GFSK_RX_STATUS_ABORT_ERR               0b00000100  //  2     2                     abort error
#define SX126X_GFSK_RX_STATUS_PACKET_RECEIVED         0b00000010  //  2     2                     packet received
#define SX126X_GFSK_RX_STATUS_PACKET_SENT             0b00000001  //  2     2                     packet sent

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

/* DIO1 IRQ handler */
typedef void(DioIrqHandler)(void);

#endif /* SX126X_DEFINITIONS_H */
