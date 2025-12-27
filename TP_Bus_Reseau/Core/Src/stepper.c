#include "stepper.h"

static CAN_HandleTypeDef *phcan;
static uint32_t TxMailbox;

void Stepper_Init(CAN_HandleTypeDef *hcan)
{
    phcan = hcan;
    HAL_CAN_Start(phcan);

    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8];

    TxHeader.StdId = STEPPER_CAN_ID_CONFIG; // 0x62
    TxHeader.ExtId = 0;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 0; // No data
    TxHeader.TransmitGlobalTime = DISABLE;

    HAL_CAN_AddTxMessage(phcan, &TxHeader, TxData, &TxMailbox);
    HAL_Delay(100);
}

void Stepper_SetAngle(uint8_t angle, uint8_t sign)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8];

    TxHeader.StdId = STEPPER_CAN_ID_ANGLE;
    TxHeader.ExtId = 0;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 3;
    TxHeader.TransmitGlobalTime = DISABLE;

    TxData[0] = angle;
    TxData[1] = sign;
    TxData[2] = 0x00; // Unused in Angle mode usually, but kept for safety

    HAL_CAN_AddTxMessage(phcan, &TxHeader, TxData, &TxMailbox);
    HAL_Delay(100);
}

void Stepper_RotateManual(uint8_t steps, uint8_t direction, uint8_t speed)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8];

    TxHeader.StdId = STEPPER_CAN_ID_MANUAL;
    TxHeader.ExtId = 0;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 3;
    TxHeader.TransmitGlobalTime = DISABLE;

    TxData[0] = direction;
    TxData[1] = steps;
    TxData[2] = speed;

    HAL_CAN_AddTxMessage(phcan, &TxHeader, TxData, &TxMailbox);
    HAL_Delay(100);
}
