#include "oled.h"
#include "fonts.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define OLED_ADDR 0x7A
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const uint8_t MAX_PAGES = SCREEN_HEIGHT / 8;

uint8_t GRAM[8][128] = {0};

void OLED_Send_Cmd(uint8_t cmd) {
  uint8_t data[2] = {0x00, cmd};
  HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, data, 2, 100);
}

void OLED_Init() {
  OLED_Send_Cmd(0xAE);

  OLED_Send_Cmd(0x02);
  OLED_Send_Cmd(0x10);

  OLED_Send_Cmd(0x40);

  OLED_Send_Cmd(0xB0);

  OLED_Send_Cmd(0x81);
  OLED_Send_Cmd(0xCF);

  OLED_Send_Cmd(0xA1);

  OLED_Send_Cmd(0xA6);

  OLED_Send_Cmd(0xA8);
  OLED_Send_Cmd(0x3F);

  OLED_Send_Cmd(0xAD);
  OLED_Send_Cmd(0x8B);

  OLED_Send_Cmd(0x33);

  OLED_Send_Cmd(0xC8);

  OLED_Send_Cmd(0xD3);
  OLED_Send_Cmd(0x00);

  OLED_Send_Cmd(0xD5);
  OLED_Send_Cmd(0xC0);

  OLED_Send_Cmd(0xD9);
  OLED_Send_Cmd(0x1F);

  OLED_Send_Cmd(0xDA);
  OLED_Send_Cmd(0x12);

  OLED_Send_Cmd(0xDB);
  OLED_Send_Cmd(0x40);

  OLED_Send_Cmd(0xAF);
}

void OLED_Clear() {
  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 0; j < 128; j++) {
      GRAM[i][j] = 0;
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
}

/**
 * @author Yang He
 * @brief  用于在 OLED 上显示汉字
 * @param  fonts_index: 汉字在 fonts_map 中的索引，-1 表示换行符
 * @param  number: 汉字的数量（包括换行符）
 * @param  size: 汉字的大小（例如 16 * 16），必须为 8 的倍数
 * @return 0 表示成功，-1 参数设置错误，-2 内存错误，-3 字数超过显示范围
 * @note   换行符的索引为 -1，汉字索引不可为负数，目前仅适配 16 * 16 的汉字
 */
uint8_t OLED_Show_By_Index(int16_t *fonts_index, uint8_t number, uint8_t size, uint8_t is_ascii) {
  /*--------------------参数检查-----------------------*/
  if (size % 8 != 0 || size == 0)
    return -1;

  uint8_t filled_pages = size / 8;
  uint8_t max_chars_per_line = SCREEN_WIDTH / size;
  uint8_t max_lines = SCREEN_HEIGHT / size;
  uint8_t max_chars = max_chars_per_line * max_lines;
  if (number > max_chars)
    return -3;
  /*--------------------------------------------------*/

  /*-------------------处理换行符的索引------------------*/
  uint8_t current_enter_index = 0;
  uint8_t max_enter_number = max_lines - 1;
  uint8_t *enter_index = (uint8_t *)malloc(max_enter_number * sizeof(uint8_t));
  if (enter_index == NULL)
    return -2;

  for (uint8_t i = 0; i < number; i++) {
    if (fonts_index[i] == -1) {
      if (current_enter_index >= max_enter_number) { // 换行符超过最大数量
        free(enter_index);
        return -3;
      }
      enter_index[current_enter_index++] = i;
    }
  }

  uint8_t *temp_enter_index =
      (uint8_t *)realloc(enter_index, current_enter_index * sizeof(uint8_t));
  if (temp_enter_index != NULL) {
    enter_index = temp_enter_index;
  }

  uint8_t total_lines = current_enter_index + 1;
  if (total_lines > max_lines) { // 确保行数不超过最大数量
    free(enter_index);
    return -3;
  }
  /*--------------------------------------------------*/

  /*----------------处理一行汉字写入缓冲区----------------*/
  bool is_over_screen = false;
  for (uint8_t line = 0; line < total_lines; line++) {
    uint8_t start = (line == 0) ? 0 : (enter_index[line - 1] + 1);
    uint8_t end = (line == current_enter_index) ? number : enter_index[line];
    uint8_t chars_in_line_number = end - start;

    if (chars_in_line_number > max_chars_per_line) {
      chars_in_line_number = max_chars_per_line; // 截断超出的字符
      is_over_screen = true; // 标记超出屏幕，便于返回错误
    }

    // 提取当前行的字符索引
    int8_t *line_index =
        (int8_t *)malloc(chars_in_line_number * sizeof(int8_t));
    if (line_index == NULL) {
      free(enter_index);
      return -2;
    }

    for (uint8_t i = 0; i < chars_in_line_number; i++) {
      line_index[i] = fonts_index[start + i];
    }

    /*------------------------拷贝字模数据-----------------------------*/
    uint8_t *line_font =
        (uint8_t *)malloc(chars_in_line_number * 32 *
                          sizeof(uint8_t)); // fixme: 适配不同大小的字体
    if (line_font == NULL) {
      free(line_index);
      free(enter_index);
      return -2;
    }

    for (uint8_t i = 0; i < chars_in_line_number; i++) {
      if (is_ascii) {
        memcpy(&line_font[i * 32], ascii[line_index[i]],
               32); // fixme: 适配不同大小的字体
      } else {
        memcpy(&line_font[i * 32], fonts[line_index[i]],
               32); // fixme: 适配不同大小的字体
      }
    }

    free(line_index);
    /*---------------------------------------------------------------*/

    /*--------------------将数据分页写入 GRAM---------------------------*/
    for (uint8_t page_in_line = 0; page_in_line < filled_pages;
         page_in_line++) { // 每页写入一行的数据
      uint8_t gram_page = line * filled_pages + page_in_line;
      if (gram_page >= MAX_PAGES) {
        free(line_font);
        free(enter_index);
        return -3; // 超出 GRAM 范围
      }

      for (uint8_t char_index = 0; char_index < chars_in_line_number;
           char_index++) {
        uint16_t char_offset = // 每个字符在 line_font 的偏移量
            char_index * 32 +
            page_in_line * (32 / filled_pages); // fixme: 适配不同大小的字体
        uint8_t column_start = char_index * size;
        for (uint8_t column = 0; column < size; column++) {
          if (column_start + column >= SCREEN_WIDTH)
            break;
          GRAM[gram_page][column_start + column] =
              line_font[char_offset + column];
        }
      }
    }

    free(line_font);
  }

  free(enter_index);
  /*---------------------------------------------------------------*/

  /*----------------------将GRAM数据写入OLED-------------------------*/
  for (uint8_t page = 0; page < MAX_PAGES; page++) {
    OLED_Send_Cmd(0xB0 + page);
    OLED_Send_Cmd(0x00);
    OLED_Send_Cmd(0x10);
    uint8_t buffer[129];
    buffer[0] = 0x40;
    memcpy(&buffer[1], GRAM[page], 128);
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, buffer, 129, HAL_MAX_DELAY);
  }
  /*---------------------------------------------------------------*/
  if (is_over_screen) {
    return -3;
  } else {
    return 0;
  }
}

/**
 * @author Yang He
 * @brief 用于直接在 OLED 上显示 ASCII 字符串
 * @param ascii: ASCII 字符串，语法和 printf 相同，暂时只支持 '\n' 换行
 * @param size: 字体大小，目前仅支持 16 * 16 的字体
 * @return 0 表示成功，-1 参数设置错误，-2 内存错误，-3 字数超过显示范围
 */
uint8_t OLED_Show_ASCII(char *ascii, uint8_t size) {
  /*--------------------参数检查-----------------------*/
  if (ascii == NULL || ascii[0] == '\0') {
    return -1;
  }
  /*--------------------------------------------------*/

  /*----------建立 ascii_map、fonts_map 索引------------*/
  int16_t ascii_index[256];
  int16_t fonts_index[256];
  memset(ascii_index, -1, sizeof(ascii_index));
  memset(fonts_index, -1, sizeof(fonts_index)); // fixme: 建立 ascii_map、fonts_map 共同索引

  for (uint8_t i = 0; i < strlen(ascii_map); i++) {
    unsigned char temp_ascii = (unsigned char)ascii_map[i];
    if (ascii_index[temp_ascii] == -1) {
      ascii_index[temp_ascii] = i;
    }
  }

  for (uint8_t i = 0; i < strlen(fonts_map); i++) {
    unsigned char temp_fonts = (unsigned char)fonts_map[i];
    if (fonts_index[temp_fonts] == -1) {
      fonts_index[temp_fonts] = i;
    }
  }
  /*--------------------------------------------------*/

  /*-------------------计算可用字符数量------------------*/
  uint8_t available_ascii_chars = 0;
  uint8_t available_font_chars = 0;
  uint8_t length = strlen(ascii);
  for (uint8_t i = 0; i < length; i++) {
    if (ascii[i] == '\n') {
      available_ascii_chars++;
    } else {
      if (ascii_index[(unsigned char)ascii[i]] != -1) {
        available_ascii_chars++;
      } else {
        available_font_chars++;
      }
    }
  }
  /*--------------------------------------------------*/

  /*-----------------建立 ASCII 索引--------------------*/
  if (available_ascii_chars == 0)
    return 0;
  int16_t *ascii_fonts_index = (int16_t *)malloc(available_ascii_chars * sizeof(int16_t));
  if (ascii_fonts_index == NULL)
    return -2;
  uint8_t fonts_index_index = 0;
  for (uint8_t i = 0; i < length; i++) {
    if (ascii[i] == '\n') {
      ascii_fonts_index[fonts_index_index++] = -1;
    } else {
      if (ascii_index[(unsigned char)ascii[i]] != -1) {
        ascii_fonts_index[fonts_index_index++] =
            ascii_index[(unsigned char)ascii[i]];
      }
    }
  }
  /*--------------------------------------------------*/

  uint8_t result =
      OLED_Show_By_Index(ascii_fonts_index, fonts_index_index, size, true);
  free(ascii_fonts_index);
  return result;
}

// todo: 考虑对于非 ASCII 字符的显示，在该字符前、后引入 -2 以标记该部分为非
// ASCII 字符，需要后续在 OLED_Show_By_Index 中特殊处理，OLED_Show_By_Index 同样需要修改
