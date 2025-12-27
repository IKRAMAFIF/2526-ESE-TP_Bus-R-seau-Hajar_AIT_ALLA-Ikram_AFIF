/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "bmp280.h"
#include "mpu9250.h"
#include "stepper.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t rxByte;
char rxBuffer[32];
uint8_t rxIndex = 0;
float K_coeff = 1.0f;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void ProcessCommand(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    if (rxByte == '\n' || rxByte == '\r')
    {
      rxBuffer[rxIndex] = '\0';
      if (rxIndex > 0)
      {
    	  printf("CMD->%s\r\n",rxBuffer);
    	  ProcessCommand();
      }
      rxIndex = 0;
    }
    else
    {
      if (rxIndex < 31)
      {
        rxBuffer[rxIndex++] = (char)rxByte;
      }
    }
    HAL_UART_Receive_IT(&huart1, &rxByte, 1);
  }
}

void ProcessCommand(void)
{
	char txBuffer[64];
	float temp, press;

	if (strcmp(rxBuffer, "GET_T") == 0)
	{
		BMP280_ReadTemperaturePressure(&temp, &press);
		sprintf(txBuffer, "T=%+06.2f_C\r\n", temp);
		printf(txBuffer);
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuffer, strlen(txBuffer), 100);
	}
	else if (strcmp(rxBuffer, "GET_P") == 0)
	{
		BMP280_ReadTemperaturePressure(&temp, &press);
		sprintf(txBuffer, "P=%06.0fPa\r\n", press);
		printf(txBuffer);
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuffer, strlen(txBuffer), 100);
	}
	else if (strncmp(rxBuffer, "SET_K=", 6) == 0)
	{
		int k_val;
		if (sscanf(rxBuffer + 6, "%d", &k_val) == 1)
		{
			K_coeff = k_val / 100.0f;
			sprintf(txBuffer, "SET_K=OK\r\n");
		}
		else
		{
			sprintf(txBuffer, "SET_K=ERR\r\n");
		}
		printf(txBuffer);
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuffer, strlen(txBuffer), 100);
	}
	else if (strcmp(rxBuffer, "GET_K") == 0)
	{
		sprintf(txBuffer, "K=%08.5f\r\n", K_coeff);
		printf(txBuffer);
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuffer, strlen(txBuffer), 100);
	}
	else if (strcmp(rxBuffer, "GET_A") == 0)
	{
		MPU9250_Data mpu;
		MPU9250_ReadAccel(&mpu);
		float angle = atan2f(mpu.Accel_X, sqrtf(mpu.Accel_Y * mpu.Accel_Y + mpu.Accel_Z * mpu.Accel_Z)) * 180.0f / 3.14159f;
		sprintf(txBuffer, "A=%08.4f\r\n", angle);
		printf(txBuffer);
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuffer, strlen(txBuffer), 100);
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /*float temperature, pressure;
  MPU9250_Data mpuData;*/
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_CAN1_Init();
  /* USER CODE BEGIN 2 */
  BMP280_Init();
  HAL_Delay(500);
  MPU9250_Init();

  HAL_UART_Receive_IT(&huart1, &rxByte, 1);
  Stepper_Init(&hcan1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  float temp, press;
  uint8_t angle;
  const float TEMP_TARGET = 20.0f;

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	BMP280_ReadTemperaturePressure(&temp, &press);
	
	float error = TEMP_TARGET - temp;
	float calculated_angle = error * K_coeff;

	// Clamp the angle between 0 (closed) and 90 (fully open)
	if (calculated_angle < 0.0f) {
		calculated_angle = 0.0f;
	} else if (calculated_angle > 90.0f) {
		calculated_angle = 90.0f;
	}

	angle = (uint8_t)calculated_angle;

	printf("Temp: %.2f C | Target: %.2f C | Error: %.2f | Valve Angle: %d\r\n", temp, TEMP_TARGET, error, angle);
	
	Stepper_SetAngle(angle, STEPPER_SIGN_POS);
	
    HAL_Delay(1000);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
