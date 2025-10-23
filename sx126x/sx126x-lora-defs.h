/* NOTE:
 * This file includes LoRa specific definitions
 * */

#ifndef SX126X_LORA_DEFS_H
#define SX126X_LORA_DEFS_H

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

/* Table 13-72: CAD Number of Symbol Definition */
typedef enum
{
    CAD_01_SYMBOL = 0x00,   /* 1 symbol used for CAD*/
    CAD_02_SYMBOL = 0x01,   /* 2 symbol used for CAD*/
    CAD_04_SYMBOL = 0x02,   /* 4 symbol used for CAD*/
    CAD_08_SYMBOL = 0x03,   /* 8 symbol used for CAD*/
    CAD_16_SYMBOL = 0x04,   /* 16 symbols used for CAD*/
} CAD_SYMBOLS;

/* Table 13-73: CAD Exit Mode Definition */
typedef enum
{
    CAD_ONLY = 0x00,
    CAD_RX = 0x01,
    CAD_LBT = 0x10,
} CAD_EXIT_MODES;

/* Table 13-47: LoRa® ModParam1- SF */
typedef enum
{
    SF5 = 0x05,
    SF6 = 0x06,
    SF7 = 0x07,
    SF8 = 0x08,
    SF9 = 0x09,
    SF10 = 0x0A,
    SF11 = 0x0B,
    SF12 = 0x0C,
} SPREAD_FACTORS;

/* Table 13-48: LoRa® ModParam2 - BW */
typedef enum
{
    BW_500 = 6,
    BW_250 = 5,
    BW_125 = 4,
    BW_062 = 3,
    BW_041 = 10,
    BW_031 = 2,
    BW_020 = 9,
    BW_015 = 1,
    BW_010 = 8,
    BW_007 = 0,
} BANDWIDTHS;

/* Table 13-49: LoRa® ModParam3 - CR */
typedef enum
{
    CR_4_5 = 0x01,
    CR_4_6 = 0x02,
    CR_4_7 = 0x03,
    CR_4_8 = 0x04,
} CODING_RATES;

/* Table 13-50: LoRa® ModParam4 - LowDataRateOptimize */
typedef enum
{
    OPTIMIZE_OFF = 0x00,
    OPTIMIZE_ON = 0x01,
} LOW_DATA_RATE_OPTIMIZE;

/* Table 13-67: LoRa® PacketParam3 - HeaderType */
typedef enum
{
    PACKET_EXPLICIT = 0x00,    /* Variable length packet (explicit header) */
    PACKET_IMPLICIT = 0x01,    /* Fixed length packet (implicit header) */
} HEADER_TYPES;

/* Table 13-69: LoRa® PacketParam5 - CRCType */
typedef enum
{
    CRC_ON = 0x01,
    CRC_OFF = 0x00,
} CRC_MODES;

/* Table 13-70: LoRa® PacketParam6 - InvertIQ */
typedef enum
{
    IQ_NORMAL = 0x00,
    IQ_INVERTED = 0x01,
} IQ_MODES;

#endif /* SX126X_LORA_DEFS_H */
