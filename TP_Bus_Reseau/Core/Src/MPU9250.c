#include "mpu9250.h"
#include "i2c.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;

uint8_t MPU9250_ReadID(void)
{
    uint8_t rx_data = 0;
    HAL_I2C_Mem_Read(&hi2c1, MPU9250_I2C_ADDR, MPU9250_REG_WHO_AM_I, 1, &rx_data, 1, HAL_MAX_DELAY);
    return rx_data;
}

void MPU9250_Init(void)
{
    uint8_t id = MPU9250_ReadID();
    if (id == MPU9250_WHO_AM_I_VAL)
    {
        printf("MPU9250 OK. . Initialisation en cours...\r\n", id);
        // Wake up MPU9250 (Clear SLEEP bit in PWR_MGMT_1)
        uint8_t data = 0x00;
        HAL_I2C_Mem_Write(&hi2c1, MPU9250_I2C_ADDR, MPU9250_REG_PWR_MGMT_1, 1, &data, 1, HAL_MAX_DELAY);
        data = 0x07; // 0000 0111
        HAL_I2C_Mem_Write(&hi2c1, MPU9250_I2C_ADDR, MPU9250_REG_PWR_MGMT_2, 1, &data, 1, HAL_MAX_DELAY);
    }
    else
    {
        printf("Error: MPU9250 not found (ID: 0x%02X)\r\n", id);
    }
}

void MPU9250_ReadAccel(MPU9250_Data *data)
{
    uint8_t rx_data[6];
    int16_t raw_x, raw_y, raw_z;

    HAL_I2C_Mem_Read(&hi2c1, MPU9250_I2C_ADDR, MPU9250_REG_ACCEL_XOUT_H, 1, rx_data, 6, HAL_MAX_DELAY);

    raw_x = (int16_t)((rx_data[0] << 8) | rx_data[1]);
    raw_y = (int16_t)((rx_data[2] << 8) | rx_data[3]);
    raw_z = (int16_t)((rx_data[4] << 8) | rx_data[5]);

    data->Accel_X = raw_x / 16384.0f;
    data->Accel_Y = raw_y / 16384.0f;
    data->Accel_Z = raw_z / 16384.0f;
}
