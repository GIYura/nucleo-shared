#ifndef ADXL345_H
#define ADXL345_H

#include <stdint.h>

void ADXL_Init(void);
void ADXL_GetId(uint8_t* const id);
void ADXL_GetVector(void);
void ADXL_DumpRegisters(void);

#endif /* ADXL345_H */
