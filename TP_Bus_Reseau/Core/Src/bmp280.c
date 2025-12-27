#include "bmp280.h"
#include "main.h"
#include "i2c.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;
static BMP280_CalibData calibData;
static int32_t t_fine;

uint8_t BMP280_ReadID(void)
{
    uint8_t tx_data = BMP280_REG_ID;
    uint8_t rx_data = 0;
    HAL_I2C_Master_Transmit(&hi2c1, BMP280_I2C_ADDR, &tx_data, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&hi2c1, BMP280_I2C_ADDR, &rx_data, 1, HAL_MAX_DELAY);
    return rx_data;
}

void BMP280_Config(void)
{
    uint8_t tx_data[2];
    // Write CTRL_MEAS register
    tx_data[0] = BMP280_REG_CTRL_MEAS;
    tx_data[1] = 0x57; // Normal mode, Temp x2, Press x16
    HAL_I2C_Master_Transmit(&hi2c1, BMP280_I2C_ADDR, tx_data, 2, HAL_MAX_DELAY);
}

void BMP280_ReadCalibration(void)
{
    uint8_t tx_data = BMP280_REG_CALIB_START;
    uint8_t rx_data[24];
    HAL_I2C_Master_Transmit(&hi2c1, BMP280_I2C_ADDR, &tx_data, 1, HAL_MAX_DELAY);
    // Read all 24 bytes
    HAL_I2C_Master_Receive(&hi2c1, BMP280_I2C_ADDR, rx_data, 24, HAL_MAX_DELAY);
    calibData.dig_T1 = (rx_data[1] << 8) | rx_data[0];
    calibData.dig_T2 = (int16_t)((rx_data[3] << 8) | rx_data[2]);
    calibData.dig_T3 = (int16_t)((rx_data[5] << 8) | rx_data[4]);
    calibData.dig_P1 = (rx_data[7] << 8) | rx_data[6];
    calibData.dig_P2 = (int16_t)((rx_data[9] << 8) | rx_data[8]);
    calibData.dig_P3 = (int16_t)((rx_data[11] << 8) | rx_data[10]);
    calibData.dig_P4 = (int16_t)((rx_data[13] << 8) | rx_data[12]);
    calibData.dig_P5 = (int16_t)((rx_data[15] << 8) | rx_data[14]);
    calibData.dig_P6 = (int16_t)((rx_data[17] << 8) | rx_data[16]);
    calibData.dig_P7 = (int16_t)((rx_data[19] << 8) | rx_data[18]);
    calibData.dig_P8 = (int16_t)((rx_data[21] << 8) | rx_data[20]);
    calibData.dig_P9 = (int16_t)((rx_data[23] << 8) | rx_data[22]);
}

void BMP280_Init(void)
{
    uint8_t id = BMP280_ReadID();
    printf("BMP280 found (ID: 0x%02X). Configuring...\r\n", id);
    BMP280_ReadCalibration();
    BMP280_Config();
}

void BMP280_ReadTemperaturePressure(float *temp, float *press)
{
    uint8_t tx_data = BMP280_REG_PRESS_MSB;
    uint8_t rx_data[6]; // Press_MSB, Press_LSB, Press_XLSB, Temp_MSB, Temp_LSB, Temp_XLSB
    int32_t adc_P, adc_T;
    // Read data from 0xF7
    HAL_I2C_Master_Transmit(&hi2c1, BMP280_I2C_ADDR, &tx_data, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, BMP280_I2C_ADDR, rx_data, 6, HAL_MAX_DELAY);
    adc_P = (rx_data[0] << 12) | (rx_data[1] << 4) | (rx_data[2] >> 4);
    adc_T = (rx_data[3] << 12) | (rx_data[4] << 4) | (rx_data[5] >> 4);
    // --- Temperature Compensation
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)calibData.dig_T1 << 1))) * ((int32_t)calibData.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calibData.dig_T1)) * ((adc_T >> 4) - ((int32_t)calibData.dig_T1))) >> 12) * ((int32_t)calibData.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    *temp = T / 100.0f;

    // --- Pressure Compensation
    int64_t p_var1, p_var2, p;
    p_var1 = (int64_t)t_fine - 128000;
    p_var2 = p_var1 * p_var1 * (int64_t)calibData.dig_P6;
    p_var2 = p_var2 + ((p_var1 * (int64_t)calibData.dig_P5) << 17);
    p_var2 = p_var2 + (((int64_t)calibData.dig_P4) << 35);
    p_var1 = ((p_var1 * p_var1 * (int64_t)calibData.dig_P3) >> 8) + ((p_var1 * (int64_t)calibData.dig_P2) << 12);
    p_var1 = (((((int64_t)1) << 47) + p_var1)) * ((int64_t)calibData.dig_P1) >> 33;
    p = 1048576 - adc_P;
    p = (((p << 31) - p_var2) * 3125) / p_var1;
    p_var1 = (((int64_t)calibData.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    p_var2 = (((int64_t)calibData.dig_P8) * p) >> 19;
    p = ((p + p_var1 + p_var2) >> 8) + (((int64_t)calibData.dig_P7) << 4);
    *press = (float)p / 256.0f;
}
