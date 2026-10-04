/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define Cold_Junc_Pin GPIO_PIN_3
#define Cold_Junc_GPIO_Port GPIOA
#define ThermK2_Pin GPIO_PIN_5
#define ThermK2_GPIO_Port GPIOA
#define Enc_btn2_Pin GPIO_PIN_6
#define Enc_btn2_GPIO_Port GPIOA
#define Enc_dt2_Pin GPIO_PIN_7
#define Enc_dt2_GPIO_Port GPIOA
#define Enc_clk2_Pin GPIO_PIN_0
#define Enc_clk2_GPIO_Port GPIOB
#define N_MOS_2_Pin GPIO_PIN_1
#define N_MOS_2_GPIO_Port GPIOB
#define N_MOS_3_Pin GPIO_PIN_10
#define N_MOS_3_GPIO_Port GPIOB
#define MAX6675_CS2_Pin GPIO_PIN_11
#define MAX6675_CS2_GPIO_Port GPIOB
#define MAX6675_CS1_Pin GPIO_PIN_12
#define MAX6675_CS1_GPIO_Port GPIOB
#define Enc_btn1_Pin GPIO_PIN_8
#define Enc_btn1_GPIO_Port GPIOA
#define Enc_dt1_Pin GPIO_PIN_9
#define Enc_dt1_GPIO_Port GPIOA
#define Enc_clk1_Pin GPIO_PIN_10
#define Enc_clk1_GPIO_Port GPIOA
#define N_MOS_LO_power_H_Pin GPIO_PIN_15
#define N_MOS_LO_power_H_GPIO_Port GPIOA
#define N_MOS_LO_power_L_Pin GPIO_PIN_3
#define N_MOS_LO_power_L_GPIO_Port GPIOB
#define P_MOS_Pin GPIO_PIN_4
#define P_MOS_GPIO_Port GPIOB
#define N_MOS_1_Pin GPIO_PIN_5
#define N_MOS_1_GPIO_Port GPIOB
#define LCD_RS_Pin GPIO_PIN_6
#define LCD_RS_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_7
#define LCD_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
typedef struct {
		int16_t H12enc;
		int16_t H6enc;
		int16_t H13enc;
		int16_t H10enc;
		int16_t H11enc;
		int16_t H14enc;
		uint8_t mode1;
		uint8_t mode2;
}ModeStruct;

typedef struct {
		int16_t Enc1;
		int16_t Enc2;
		int16_t T1;
		int16_t T2;
		int16_t T3;
		int16_t T4;
		int16_t Tkj;
		int16_t ADC;
		uint8_t PrintFlag;
		ModeStruct *mode;
}DisplayInfo;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
