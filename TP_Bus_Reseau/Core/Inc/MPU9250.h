#ifndef MPU9250_H
#define MPU9250_H

#include "main.h"

#define MPU9250_ADDR         0x68 << 1   // AD0 = 0
#define MPU_WHOAMI_REG       0x75
#define MPU_WHOAMI_ANSWER    0x71

#define MPU_PWR_MGMT_1       0x6B
#define MPU_ACCEL_XOUT_H     0x3B

int MPU9250_Init(I2C_HandleTypeDef *hi2c);
int MPU9250_Read_Accel(I2C_HandleTypeDef *hi2c, int16_t *Ax, int16_t *Ay, int16_t *Az);

#endif
