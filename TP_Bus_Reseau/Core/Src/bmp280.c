#include "bmp280.h"
#include <stdio.h>


// Lire un registre simple
HAL_StatusTypeDef BMP280_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *value)
{
    if (HAL_I2C_Master_Transmit(hi2c, BMP280_I2C_ADDRESS, &reg, 1, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    if (HAL_I2C_Master_Receive(hi2c, BMP280_I2C_ADDRESS, value, 1, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    return HAL_OK;
}


// Lire l'ID BMP280
HAL_StatusTypeDef BMP280_ReadID(I2C_HandleTypeDef *hi2c, uint8_t *id)
{
    return BMP280_ReadRegister(hi2c, BMP280_ID_REG, id);
}


// Configurer BMP280 : T×2, P×16, mode normal
HAL_StatusTypeDef BMP280_Config(I2C_HandleTypeDef *hi2c)
{
    uint8_t config_data[2];
    config_data[0] = BMP280_CTRL_MEAS;  // registre 0xF4
    config_data[1] = 0x57;              // T×2, P×16, mode normal

    if (HAL_I2C_Master_Transmit(hi2c, BMP280_I2C_ADDRESS, config_data, 2, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    return HAL_OK;
}


// Lire les 24 octets d'étalonnage
HAL_StatusTypeDef BMP280_ReadCalibration(I2C_HandleTypeDef *hi2c, BMP280_CalibData *calib)
{
    uint8_t calib_data[24];
    uint8_t start_addr = 0x88;

    if (HAL_I2C_Master_Transmit(hi2c, BMP280_I2C_ADDRESS, &start_addr, 1, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    if (HAL_I2C_Master_Receive(hi2c, BMP280_I2C_ADDRESS, calib_data, 24, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    // format little-endian
    calib->dig_T1 = (uint16_t)(calib_data[1] << 8 | calib_data[0]);
    calib->dig_T2 = (int16_t)(calib_data[3] << 8 | calib_data[2]);
    calib->dig_T3 = (int16_t)(calib_data[5] << 8 | calib_data[4]);

    calib->dig_P1 = (uint16_t)(calib_data[7] << 8 | calib_data[6]);
    calib->dig_P2 = (int16_t)(calib_data[9] << 8 | calib_data[8]);
    calib->dig_P3 = (int16_t)(calib_data[11] << 8 | calib_data[10]);
    calib->dig_P4 = (int16_t)(calib_data[13] << 8 | calib_data[12]);
    calib->dig_P5 = (int16_t)(calib_data[15] << 8 | calib_data[14]);
    calib->dig_P6 = (int16_t)(calib_data[17] << 8 | calib_data[16]);
    calib->dig_P7 = (int16_t)(calib_data[19] << 8 | calib_data[18]);
    calib->dig_P8 = (int16_t)(calib_data[21] << 8 | calib_data[20]);
    calib->dig_P9 = (int16_t)(calib_data[23] << 8 | calib_data[22]);

    return HAL_OK;
}


// Lire température + pression brutes
HAL_StatusTypeDef BMP280_ReadRawValues(I2C_HandleTypeDef *hi2c, int32_t *raw_temp, int32_t *raw_press)
{
    uint8_t start = 0xF7;
    uint8_t data[6];

    if (HAL_I2C_Master_Transmit(hi2c, BMP280_I2C_ADDRESS, &start, 1, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    if (HAL_I2C_Master_Receive(hi2c, BMP280_I2C_ADDRESS, data, 6, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    // Pression 20 bits
    *raw_press = ((int32_t)data[0] << 12) |
                 ((int32_t)data[1] << 4)  |
                 ( data[2] >> 4 );

    // Température 20 bits
    *raw_temp = ((int32_t)data[3] << 12) |
                ((int32_t)data[4] << 4)  |
                ( data[5] >> 4 );

    return HAL_OK;
}
static int32_t t_fine;

int32_t BMP280_CompensateTemperature(int32_t adc_T, BMP280_CalibData *cal)
{
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)cal->dig_T1 << 1)))
            * ((int32_t)cal->dig_T2)) >> 11;

    var2 = (((((adc_T >> 4) - ((int32_t)cal->dig_T1))
            * ((adc_T >> 4) - ((int32_t)cal->dig_T1))) >> 12)
            * ((int32_t)cal->dig_T3)) >> 14;

    t_fine = var1 + var2;

    T = (t_fine * 5 + 128) >> 8;    // température ×100

    return T;  // Exemple 2510 = 25.10°C
}
uint32_t BMP280_CompensatePressure(int32_t adc_P, BMP280_CalibData *cal)
{
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)cal->dig_P6;
    var2 = var2 + ((var1 * (int64_t)cal->dig_P5) << 17);
    var2 = var2 + (((int64_t)cal->dig_P4) << 35);

    var1 = ((var1 * var1 * (int64_t)cal->dig_P3) >> 8) +
           ((var1 * (int64_t)cal->dig_P2) << 12);

    var1 = (((((int64_t)1) << 47) + var1) * ((int64_t)cal->dig_P1)) >> 33;

    if (var1 == 0)
        return 0; // avoid error

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;

    var1 = (((int64_t)cal->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)cal->dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)cal->dig_P7) << 4);
    p=p/256;
    return (uint32_t)p;   // Pa (pression réelle)
}

