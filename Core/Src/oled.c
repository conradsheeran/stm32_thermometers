#include "oled.h"
#include "fonts.h"

#define OLED_ADDR 0x7A

uint8_t GRAM[8][128] = {0};

void OLED_Send_Cmd(uint8_t cmd) {
  uint8_t data[2] = {0x00, cmd};
  HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, data, 2, 100);
}

void OLED_Init() {
  OLED_Send_Cmd(0xAE); /*关闭显示 display off*/

  OLED_Send_Cmd(0x02); /*设置列起始地址 set lower column address*/
  OLED_Send_Cmd(0x10); /*设置列结束地址 set higher column address*/

  OLED_Send_Cmd(0x40); /*设置起始行 set display start line*/

  OLED_Send_Cmd(0xB0); /*设置页地址 set page address*/

  OLED_Send_Cmd(0x81); /*设置对比度 contract control*/
  OLED_Send_Cmd(0xCF); /*128*/

  OLED_Send_Cmd(0xA1); /*设置分段重映射 从右到左 set segment remap*/

  OLED_Send_Cmd(0xA6); /*正向显示 normal / reverse*/

  OLED_Send_Cmd(0xA8); /*多路复用率 multiplex ratio*/
  OLED_Send_Cmd(0x3F); /*duty = 1/64*/

  OLED_Send_Cmd(0xAD); /*设置启动电荷泵 set charge pump enable*/
  OLED_Send_Cmd(0x8B); /*启动DC-DC */

  OLED_Send_Cmd(0x33); /*设置泵电压 set VPP 10V */

  OLED_Send_Cmd(0xC8); /*设置输出扫描方向 COM[N-1]到COM[0] Com scan direction*/

  OLED_Send_Cmd(0xD3); /*设置显示偏移 set display offset*/
  OLED_Send_Cmd(0x00); /* 0x00 */

  OLED_Send_Cmd(0xD5); /*设置内部时钟频率 set osc frequency*/
  OLED_Send_Cmd(0xC0);

  OLED_Send_Cmd(0xD9); /*设置放电/预充电时间 set pre-charge period*/
  OLED_Send_Cmd(0x1F); /*0x22*/

  OLED_Send_Cmd(0xDA); /*设置引脚布局 set COM pins*/
  OLED_Send_Cmd(0x12);

  OLED_Send_Cmd(0xDB); /*设置电平 set vcomh*/
  OLED_Send_Cmd(0x40);

  OLED_Send_Cmd(0xAF); /*开启显示 display ON*/
}

/**
 * @author Yang He
 * @brief  用于在OLED上显示汉字
 * @param  fonts_index: 汉字在 fonts_map 中的索引
 * @param  number: 汉字的数量
 * @param  size: 汉字的大小（例如 16 * 16），必须为 8 的倍数
 * @retval 返回值为 0 时表示显示成功，为 -1 时表示字数超出显示上限
 * @note   换行符的索引为 -1，故汉字索引不可使用负数
 */
uint8_t OLED_Show(int8_t *fonts_index, uint8_t number, uint8_t size) {
  if (number > 32) {  //fixme: 修复字数超出显示上限
    return -1;
  }

  uint8_t temp[][128] = {0};
  uint8_t filled_pages = size / 8;  //fixme: 修复字体大小不为 8 的倍数  和  修复 size 小于 8 的情况
  for (uint8_t i = 0; i < number; i++) {
    if (fonts_index[i] == -1) {
      continue; //fixme: 换行符
    }

    for (uint8_t j = 0; j < filled_pages; j++) {
      for (uint8_t k = 0; k < 8; k++) {
        temp[k][i * filled_pages + j] = fonts_map[fonts_index[i]][j * 8 + k];
      }
    }
  }


  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 0; j < size; j++) {
      GRAM[i][j] = temp[i][i * size + j];
    }
  }

  for (uint8_t i = 0; i < 8; i++) {
    uint8_t page = 0xB0 + i;
    OLED_Send_Cmd(page);
    OLED_Send_Cmd(0x02);
    OLED_Send_Cmd(0x10);
    uint8_t temp[129] = {0x40};
    for (uint8_t j = 0; j < 129; j++) {
      temp[j + 1] = GRAM[i][j];
    }
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, temp, 129, HAL_MAX_DELAY);
  }

  return 0;
}
