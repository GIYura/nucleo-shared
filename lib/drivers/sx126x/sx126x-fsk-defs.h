/* NOTE:
 * This file includes FSK modem specific definitions
 * */
#ifndef SX126X_FSK_DEFS_H
#define SX126X_FSK_DEFS_H

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

//SX126X_CMD_GET_PACKET_STATUS
#define SX126X_GFSK_RX_STATUS_PREAMBLE_ERR            0b10000000  //  7     7     GFSK Rx status: preamble error
#define SX126X_GFSK_RX_STATUS_SYNC_ERR                0b01000000  //  6     6                     sync word error
#define SX126X_GFSK_RX_STATUS_ADRS_ERR                0b00100000  //  5     5                     address error
#define SX126X_GFSK_RX_STATUS_CRC_ERR                 0b00010000  //  4     4                     CRC error
#define SX126X_GFSK_RX_STATUS_LENGTH_ERR              0b00001000  //  3     3                     length error
#define SX126X_GFSK_RX_STATUS_ABORT_ERR               0b00000100  //  2     2                     abort error
#define SX126X_GFSK_RX_STATUS_PACKET_RECEIVED         0b00000010  //  2     2                     packet received
#define SX126X_GFSK_RX_STATUS_PACKET_SENT             0b00000001  //  2     2                     packet sent

/* Table 13-44: GFSK ModParam4 - PulseShape */
typedef enum
{
    PULSE_SHAPING_OFF = 0x00,
    PULSE_SHAPING_G_BT_03 = 0x08,
    PULSE_SHAPING_G_BT_05 = 0x09,
    PULSE_SHAPING_G_BT_07 = 0x0A,
    PULSE_SHAPING_G_BT_1 = 0x0B,
} PULSE_SHAPINGS;

/* Table 13-53: GFSK PacketParam3 - PreambleDetectorLength */
typedef enum
{
    PREAMBLE_DETECTOR_OFF = 0x00,     //!< Preamble detection length off
    PREAMBLE_DETECTOR_08_BITS = 0x04, //!< Preamble detection length 8 bits
    PREAMBLE_DETECTOR_16_BITS = 0x05, //!< Preamble detection length 16 bits
    PREAMBLE_DETECTOR_24_BITS = 0x06, //!< Preamble detection length 24 bits
    PREAMBLE_DETECTOR_32_BITS = 0x07, //!< Preamble detection length 32 bit
} PREAMBLE_DETECTOR_LEN;

/* Table 13-56: GFSK PacketParam5 - AddrComp */
typedef enum
{
    ADDRESSCOMP_FILT_OFF = 0x00,
    ADDRESSCOMP_FILT_NODE = 0x01,
    ADDRESSCOMP_FILT_NODE_BROAD = 0x02,
} ADDR_COMP;

/* Table 13-59: GFSK PacketParam6 - PacketType */
typedef enum
{
    PACKET_FIXED_LENGTH = 0x00,     /* The packet length is known on both sides, the size of the payload is not added to the packet */
    PACKET_VARIABLE_LENGTH = 0x01,  /* The packet is on variable size, the first byte of the payload will be the size of the packet */
} PACKET_LEN;

/* Table 6-6: CRC Type Configuration */
typedef enum
{
    CRC_OFF = 0x01, //!< No CRC in use
    CRC_1_BYTES = 0x00,
    CRC_2_BYTES = 0x02,
    CRC_1_BYTES_INV = 0x04,
    CRC_2_BYTES_INV = 0x06,
    CRC_2_BYTES_IBM = 0xF1,
    CRC_2_BYTES_CCIT = 0xF2,
} CRC_TYPES;

/* Table 13-64: GFSK PacketParam9 - Whitening */
typedef enum
{
    WHITENING_DISABLE = 0x00,
    WHITENING_ENABLE = 0x01,
} WHITENING;

#endif /* SX126X_FSK_DEFS_H */
