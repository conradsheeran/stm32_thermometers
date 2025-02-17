#include "aht20.h"

#define AHT20_ADDR 0x38
#define TX_ADDR (AHT20_ADDR << 1)
#define RX_ADDR ((AHT20_ADDR << 1) | 0x01)

uint8_t rx_data[6] = {0};

void AHT20_Init(void) {
  uint8_t rx_data;
  HAL_Delay(45);

  HAL_I2C_Master_Receive(&hi2c1, RX_ADDR, &rx_data, 1, 100);

  if ((rx_data & 0x08) != 0x08) {
    uint8_t tx_data[3] = {0xBE, 0x08, 0x00};
    HAL_I2C_Master_Transmit(&hi2c1, TX_ADDR, tx_data, 3, 100);
  }
}

void AHT20_Measure(void) {
  uint8_t tx_data[3] = {0xAC, 0x33, 0x00};

  HAL_I2C_Master_Transmit_DMA(&hi2c1, TX_ADDR, tx_data, 3);
}

void AHT20_Read(void) {
  HAL_Delay(75);
  HAL_I2C_Master_Receive_DMA(&hi2c1, RX_ADDR, rx_data, 6);
}

void AHT20_Anaslyze(float *humidity, float *temperature) {
  uint8_t status = 1;

  while (status) {
    if ((rx_data[0] & 0x80) == 0x00) {
      uint32_t raw_humidity =
          ((rx_data[1] << 16) | (rx_data[2] << 8) | rx_data[3]) >> 4;
      uint32_t raw_temperature =
          ((rx_data[3] & 0x0F) << 16) | (rx_data[4] << 8) | rx_data[5];

      *humidity = ((float)raw_humidity / 1048576.0f) * 100.0f;
      *temperature = ((float)raw_temperature / 1048576.0f) * 200.0f - 50.0f;
      status = 0;
      break;
    } else {
      status = 1;
      HAL_Delay(100);
    }
  }
}
