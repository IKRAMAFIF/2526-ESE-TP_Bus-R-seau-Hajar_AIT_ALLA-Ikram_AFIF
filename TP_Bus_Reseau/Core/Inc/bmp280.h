#ifndef BMP280_H
#define BMP280_H

#include "stm32f4xx_hal.h"

// Adresse I2C (0x77 décalé à gauche pour HAL)
#define BMP280_I2C_ADDRESS   (0x77 << 1)

// Registres importants
#define BMP280_ID_REG        0xD0
#define BMP280_ID_VAL        0x58
#define BMP280_CTRL_MEAS     0xF4

// Structure des coefficients d'étalonnage
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} BMP280_CalibData;
int32_t BMP280_CompensateTemperature(int32_t adc_T, BMP280_CalibData *cal);
uint32_t BMP280_CompensatePressure(int32_t adc_P, BMP280_CalibData *cal);

// Prototypes
HAL_StatusTypeDef BMP280_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *value);
HAL_StatusTypeDef BMP280_ReadID(I2C_HandleTypeDef *hi2c, uint8_t *id);
HAL_StatusTypeDef BMP280_Config(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BMP280_ReadCalibration(I2C_HandleTypeDef *hi2c, BMP280_CalibData *calib);
HAL_StatusTypeDef BMP280_ReadRawValues(I2C_HandleTypeDef *hi2c, int32_t *raw_temp, int32_t *raw_press);

#endif
