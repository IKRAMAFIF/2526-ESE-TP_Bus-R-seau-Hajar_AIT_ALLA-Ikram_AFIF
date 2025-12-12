#ifndef __MPU9250_H
#define __MPU9250_H

#include "main.h"

// MPU9250 I2C Address
#define MPU9250_I2C_ADDR  (0x68 << 1)

#define MPU9250_REG_WHO_AM_I  0x75
#define MPU9250_REG_PWR_MGMT_1 0x6B
#define MPU9250_REG_PWR_MGMT_2 0x6C
#define MPU9250_REG_ACCEL_CFG 0x1C
#define MPU9250_REG_ACCEL_XOUT_H 0x3B
#define MPU9250_WHO_AM_I_VAL  0x71

typedef struct {
    float Accel_X;
    float Accel_Y;
    float Accel_Z;
} MPU9250_Data;

uint8_t MPU9250_ReadID(void);
void MPU9250_Init(void);
void MPU9250_ReadAccel(MPU9250_Data *data);

#endif
