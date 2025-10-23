#ifndef SX126x_REGS_H
#define SX126x_REGS_H

/* SX126X register map */
#define SX126X_REG_WHITENING_INITIAL_MSB                0x06B8
#define SX126X_REG_WHITENING_INITIAL_LSB                0x06B9
#define SX126X_REG_CRC_INITIAL_MSB                      0x06BC
#define SX126X_REG_CRC_INITIAL_LSB                      0x06BD
#define SX126X_REG_CRC_POLYNOMIAL_MSB                   0x06BE
#define SX126X_REG_CRC_POLYNOMIAL_LSB                   0x06BF
#define SX126X_REG_SYNC_WORD_0                          0x06C0
#define SX126X_REG_SYNC_WORD_1                          0x06C1
#define SX126X_REG_SYNC_WORD_2                          0x06C2
#define SX126X_REG_SYNC_WORD_3                          0x06C3
#define SX126X_REG_SYNC_WORD_4                          0x06C4
#define SX126X_REG_SYNC_WORD_5                          0x06C5
#define SX126X_REG_SYNC_WORD_6                          0x06C6
#define SX126X_REG_SYNC_WORD_7                          0x06C7
#define SX126X_REG_NODE_ADDRESS                         0x06CD
#define SX126X_REG_BROADCAST_ADDRESS                    0x06CE
#define SX126X_REG_LORA_SYNC_WORD_MSB                   0x0740
#define SX126X_REG_LORA_SYNC_WORD_LSB                   0x0741
#define SX126X_REG_RANDOM_NUMBER_0                      0x0819
#define SX126X_REG_RANDOM_NUMBER_1                      0x081A
#define SX126X_REG_RANDOM_NUMBER_2                      0x081B
#define SX126X_REG_RANDOM_NUMBER_3                      0x081C
#define SX126X_REG_TX_CLAMP_CONFIGURATION               0x08D8
#define SX126X_REG_RX_GAIN                              0x08AC
#define SX126X_REG_OCP_CONFIGURATION                    0x08E7
#define SX126X_REG_TX_MODULATION                        0x0889
#define SX126X_REG_XTA_TRIM                             0x0911
#define SX126X_REG_XTB_TRIM                             0x0912
#define SX126X_REG_RX_GAIN                              0x08AC
#define SX126X_REG_RTC_CONTROL                          0x0902
#define SX126X_REG_IRQ_POLARITY                         0x0736

#endif /* SX126x_REGS_H */