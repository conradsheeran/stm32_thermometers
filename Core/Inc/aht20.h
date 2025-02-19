#ifndef _AHT20_H
#define _AHT20_H

#include "i2c.h"

void AHT20_Init(void);
void AHT20_Measure(void);
void AHT20_Read(void);
void AHT20_Anaslyze(float *humidity, float *temperature);

#endif
