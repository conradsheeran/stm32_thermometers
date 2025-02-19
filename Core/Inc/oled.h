#ifndef OLED_H
#define OLED_H

#include "i2c.h"
void OLED_Init();
void OLED_Clear();
uint8_t OLED_Show_By_Index(int16_t *fonts_index, uint8_t number, uint8_t size,
                           uint8_t is_ascii);
uint8_t OLED_Show_ASCII(char *ascii, uint8_t size);

#endif
