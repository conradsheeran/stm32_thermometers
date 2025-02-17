#ifndef OLED_H
#define OLED_H

#include "i2c.h"
void OLED_Init();
uint8_t OLED_Show(int8_t *fonts_index, uint8_t number, uint8_t size);

#endif
