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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OnboardLED_Pin GPIO_PIN_13
#define OnboardLED_GPIO_Port GPIOC
#define PAGE_TOGGLE_Pin GPIO_PIN_0
#define PAGE_TOGGLE_GPIO_Port GPIOA
#define ALARM_TOGGLE_Pin GPIO_PIN_1
#define ALARM_TOGGLE_GPIO_Port GPIOA
#define HOUR_FORMAT_CHANGE_Pin GPIO_PIN_2
#define HOUR_FORMAT_CHANGE_GPIO_Port GPIOA
#define EDIT_CHOICE_Pin GPIO_PIN_3
#define EDIT_CHOICE_GPIO_Port GPIOA
#define INCREMENT_EDITED_VALUE_Pin GPIO_PIN_4
#define INCREMENT_EDITED_VALUE_GPIO_Port GPIOA
#define Pin_RS_Pin GPIO_PIN_12
#define Pin_RS_GPIO_Port GPIOB
#define Pin_RW_Pin GPIO_PIN_13
#define Pin_RW_GPIO_Port GPIOB
#define Pin_EN_Pin GPIO_PIN_14
#define Pin_EN_GPIO_Port GPIOB
#define ALARM_SOUND_Pin GPIO_PIN_9
#define ALARM_SOUND_GPIO_Port GPIOA
#define Pin_D0_Pin GPIO_PIN_10
#define Pin_D0_GPIO_Port GPIOA
#define Pin_D1_Pin GPIO_PIN_11
#define Pin_D1_GPIO_Port GPIOA
#define Pin_D2_Pin GPIO_PIN_12
#define Pin_D2_GPIO_Port GPIOA
#define Pin_D3_Pin GPIO_PIN_15
#define Pin_D3_GPIO_Port GPIOA
#define Pin_D4_Pin GPIO_PIN_3
#define Pin_D4_GPIO_Port GPIOB
#define Pin_D5_Pin GPIO_PIN_4
#define Pin_D5_GPIO_Port GPIOB
#define Pin_D6_Pin GPIO_PIN_5
#define Pin_D6_GPIO_Port GPIOB
#define Pin_D7_Pin GPIO_PIN_8
#define Pin_D7_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
