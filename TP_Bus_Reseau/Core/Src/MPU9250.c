#include "MPU9250.h"
#include "stdio.h"

int MPU9250_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t check = 0;
    uint8_t data = 0;

    // Lire WHO_AM_I (page 32 du datasheet) :contentReference[oaicite:1]{index=1}
    HAL_I2C_Mem_Read(hi2c, MPU9250_ADDR, MPU_WHOAMI_REG, 1, &check, 1, 100);

    if (check != MPU_WHOAMI_ANSWER) {
        printf("MPU9250 non detecte ! (0x%x)\r\n", check);
        return 1;
    }

    printf("MPU9250 detecte OK !\r\n");

    // Sortir du sleep
    data = 0x00;
    HAL_I2C_Mem_Write(hi2c, MPU9250_ADDR, MPU_PWR_MGMT_1, 1, &data, 1, 100);

    return 0;
}


int MPU9250_Read_Accel(I2C_HandleTypeDef *hi2c, int16_t *Ax, int16_t *Ay, int16_t *Az)
{
    uint8_t buf[6];

    // Lire 6 octets depuis ACCEL_XOUT_H (page 30) :contentReference[oaicite:2]{index=2}
    HAL_I2C_Mem_Read(hi2c, MPU9250_ADDR, MPU_ACCEL_XOUT_H, 1, buf, 6, 100);

    *Ax = (int16_t)(buf[0] << 8 | buf[1]);
    *Ay = (int16_t)(buf[2] << 8 | buf[3]);
    *Az = (int16_t)(buf[4] << 8 | buf[5]);

    return 0;
}
