#ifndef __STEPPER_H
#define __STEPPER_H

#include "stm32f4xx_hal.h"

// CAN IDs
#define STEPPER_CAN_ID_MANUAL 0x60
#define STEPPER_CAN_ID_ANGLE  0x61
#define STEPPER_CAN_ID_CONFIG 0x62

// Directions
#define STEPPER_DIR_CW  0x01 // Clockwise
#define STEPPER_DIR_CCW 0x00 // Counter-Clockwise

// Signs for Angle Mode
#define STEPPER_SIGN_POS 0x00
#define STEPPER_SIGN_NEG 0x01

// Function Prototypes
void Stepper_Init(CAN_HandleTypeDef *hcan);
void Stepper_SetAngle(uint8_t angle, uint8_t sign);
void Stepper_RotateManual(uint8_t steps, uint8_t direction, uint8_t speed);

#endif /* __STEPPER_H */
