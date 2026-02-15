/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd_HD44780U.h"
#include "display_control.h"
#include "debounced_button.h"
#include "ds3231.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <editor.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define N_BUTTONS_IN_CIRCUIT		5 //Total number of buttons in the circuit to be debounced
//Indices of the buttons in the debouncable buttons array
#define ALARM_TOGGLE_BUTTON_INDEX   			0
#define PAGE_TOGGLE_BUTTON_INDEX 				1
#define HOUR_FORMAT_CHANGE_BUTTON_INDEX			2
#define EDIT_CHOICE_BUTTON_INDEX				3
#define INCREMENT_EDITED_VALUE_BUTTON_INDEX		4
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
static uint8_t currentCentury = 21;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
//Sets the I2C pins as GPIO pins, tells the slaves to set the I2C bus free,
//sets the pin states back to Reset_State.
static void ResetI2CBus(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE(); //I2C Channel 1 are on pins PB6 (SCL) and PB7 (SDA)

    /*Configure GPIO pin Output Level */
    uint16_t pinSCL = GPIO_PIN_6, pinSDA = GPIO_PIN_7;
    GPIO_TypeDef* gpioChannel = GPIOB;
    //This is GPIO_PIN_RESET bc auto-generated code does it this way
    //and idk if GPIO_PIN_SET will be correct here
    HAL_GPIO_WritePin(gpioChannel, pinSCL | pinSDA, GPIO_PIN_RESET);

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    GPIO_InitStruct.Pin = pinSCL | pinSDA;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(gpioChannel, &GPIO_InitStruct);

    //Release the SDA and SCL pins because we set both of them to LOW previously.
    HAL_GPIO_WritePin(gpioChannel, pinSCL | pinSDA, GPIO_PIN_SET);

    //9 iterations because 1 byte (8 bits) of data + 1 ACK bit
    for (int i = 0; i < 9; i++)
    {
    	HAL_GPIO_WritePin(gpioChannel, pinSCL, GPIO_PIN_RESET);
		HAL_Delay(1);
		//SCL pin must be set to HIGH last, because in order for SDA being high to mean the bus
		//is released, SCL needs to be high as well. Only both lines being high means the bus
		//is free.
		HAL_GPIO_WritePin(gpioChannel, pinSCL, GPIO_PIN_SET);
		HAL_Delay(1);

		//We can stop once SDA is HIGH (bus is released)
		if (HAL_GPIO_ReadPin(gpioChannel, pinSDA) != GPIO_PIN_SET)
		{
			break;
		}
    }

    //At this point the bus is free, generate a START condition.
    //START condition is pulling SDL low while SCL is high. At this point in code,
    //SCL pin is left high. All we need to do is pull SDA low.
    HAL_GPIO_WritePin(gpioChannel, pinSDA, GPIO_PIN_RESET);

    //Generating the START condition is enough for DS3231 to know things are reset
    //but just to better comply with I2C, generate a STOP condition as well.
    //SCL is still HIGH, all the STOP condition needs is releasing SDA as well so that
    //the pull-ups will pull it high
    HAL_Delay(1);
    HAL_GPIO_WritePin(gpioChannel, pinSDA, GPIO_PIN_SET);

    //Reset the pins to their reset states
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(gpioChannel, &GPIO_InitStruct);
    __HAL_RCC_GPIOB_CLK_DISABLE(); //I2C Channel 1 are on pins PB6 (SCL) and PB7 (SDA)
}

/*
  All DS3231 I2C communication functions return the result of the communication. Instead of writing the code
  for displaying the error message after every DS3231 function call, you can simply pass the call into this
  function and it will handle the communication error displaying. It also returns the communication result
  back so if you need to do something special in your context, you can still execute your logic as well.
  In order to ensure the error message will be displayed, this function will block all other execution for
  3000ms.
*/
static HAL_StatusTypeDef I2C_ErrorHandler(HAL_StatusTypeDef commResult)
{
	if (commResult != HAL_OK)
	{
		ClearScreen();
		MoveCursor(1, 1);
		char msg[16] = { 0 };
		snprintf(msg, 16, "I2C err (%d)", commResult);
		WriteString(msg);
		HAL_Delay(3000);
	}
	return commResult;
}

static void ToggleAlarm(void)
{
	uint8_t alarmEnabled = 0;
	I2C_ErrorHandler(DS3231_IsAlarmEnabled(&alarmEnabled));
	I2C_ErrorHandler(DS3231_ToggleAlarm(!alarmEnabled));
}

static void ToggleHourFormat(void)
{
	uint8_t currentlyIn12hrFormat = 0;
	I2C_ErrorHandler(DS3231_Is12hrFormatEnabled(&currentlyIn12hrFormat));
	I2C_ErrorHandler(DS3231_SetTimeFormat(!currentlyIn12hrFormat));
}

static void SetDateForDS3231(uint16_t year, uint8_t month, uint8_t dayOfTheMonth, uint8_t dayOfTheWeek, uint8_t hoursIn24hFormat, uint8_t minutes, uint8_t seconds)
{
	I2C_ErrorHandler(DS3231_WriteYear((uint8_t)(year % 100)));
	I2C_ErrorHandler(DS3231_WriteMonth(month));
	I2C_ErrorHandler(DS3231_WriteDayOfTheMonth(dayOfTheMonth));
	I2C_ErrorHandler(DS3231_WriteDayOfTheWeek(dayOfTheWeek));
	I2C_ErrorHandler(DS3231_WriteHours(hoursIn24hFormat));
	I2C_ErrorHandler(DS3231_WriteMinutes(minutes));
	I2C_ErrorHandler(DS3231_WriteSeconds(seconds));
}

static void ReadDS3231DataIntoDisplayInfo(DisplayInfo* info)
{
	//Clock info
	I2C_ErrorHandler(DS3231_ReadSeconds(&info->seconds));
	I2C_ErrorHandler(DS3231_ReadMinutes(&info->minutes));
	I2C_ErrorHandler(DS3231_ReadHours(&info->hours, NULL, NULL));
	I2C_ErrorHandler(DS3231_IsTimePM(&info->isTimePM));
	I2C_ErrorHandler(DS3231_Is12hrFormatEnabled(&info->displayFormat));

	//Date info
	I2C_ErrorHandler(DS3231_ReadDayOfTheWeek(&info->dayOfTheWeek));
	I2C_ErrorHandler(DS3231_ReadDayOfTheMonth(&info->dayOfTheMonth));
	I2C_ErrorHandler(DS3231_ReadMonth(&info->month));

	uint8_t centuryPassed = 0;
	I2C_ErrorHandler(DS3231_ReadCenturyBit(&centuryPassed));
	if (centuryPassed)
	{
	  currentCentury++;
	  I2C_ErrorHandler(DS3231_WriteCenturyBit(0)); //Clear the century bit
	}

	uint8_t ds3231_yearInfo = 0; //Range is between 00-99
	I2C_ErrorHandler(DS3231_ReadYear(&ds3231_yearInfo));
	info->year = ((currentCentury - 1) * 100) + ds3231_yearInfo;

	//Alarm info
	I2C_ErrorHandler(DS3231_IsAlarmEnabled(&info->alarmEnabled));
	I2C_ErrorHandler(DS3231_ReadAlarmTime(&info->alarmHours, &info->alarmMinutes, &info->alarmDisplayFormat, &info->isAlarmTimePM));
	I2C_ErrorHandler(DS3231_ReadTemperature(&info->temperature));
}

static void WriteDispInfoDataIntoDS3231(const DisplayInfo* info)
{
	I2C_ErrorHandler(DS3231_WriteHours(info->hours));
	I2C_ErrorHandler(DS3231_WriteMinutes(info->minutes));
	I2C_ErrorHandler(DS3231_WriteSeconds(info->seconds));
	I2C_ErrorHandler(DS3231_SetTimeFormat(info->displayFormat));
	I2C_ErrorHandler(DS3231_WriteDayOfTheMonth(info->dayOfTheMonth));
	I2C_ErrorHandler(DS3231_WriteMonth(info->month));
	//The last 2 digits of the year are held in DS3231.
	//The higher 2 digits are based on the century flag and are calculated elsewhere.
	I2C_ErrorHandler(DS3231_WriteYear(info->year % 100));
	I2C_ErrorHandler(DS3231_WriteDayOfTheWeek(info->dayOfTheWeek));
	//TODO: Handle setting AM/PM information

	uint8_t alarmTimeIn24hFormat = 0;
	if (info->isAlarmTimePM)
	{
		if (info->alarmHours != 12)
		{
			alarmTimeIn24hFormat = info->alarmHours + 12;
		}
	}
	else
	{
		if (info->alarmHours == 12)
		{
			alarmTimeIn24hFormat = 0;
		}
	}

	I2C_ErrorHandler(DS3231_SetAlarmTime(info->alarmDisplayFormat == DISPLAY_FORMAT_12H ? alarmTimeIn24hFormat : info->alarmHours, info->alarmMinutes));
	I2C_ErrorHandler(DS3231_ToggleAlarm(info->alarmEnabled));
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  DebouncedButton buttons[N_BUTTONS_IN_CIRCUIT] = { 0 };
  buttons[ALARM_TOGGLE_BUTTON_INDEX] = InitButtonWithDefaults(ALARM_TOGGLE_GPIO_Port, ALARM_TOGGLE_Pin, GPIO_PIN_SET);
  buttons[PAGE_TOGGLE_BUTTON_INDEX] = InitButtonWithDefaults(PAGE_TOGGLE_GPIO_Port, PAGE_TOGGLE_Pin, GPIO_PIN_SET);
  buttons[HOUR_FORMAT_CHANGE_BUTTON_INDEX] = InitButtonWithDefaults(HOUR_FORMAT_CHANGE_GPIO_Port, HOUR_FORMAT_CHANGE_Pin, GPIO_PIN_SET);
  buttons[EDIT_CHOICE_BUTTON_INDEX] = InitButtonWithDefaults(EDIT_CHOICE_GPIO_Port, EDIT_CHOICE_Pin, GPIO_PIN_SET);
  buttons[INCREMENT_EDITED_VALUE_BUTTON_INDEX] = InitButtonWithDefaults(INCREMENT_EDITED_VALUE_GPIO_Port, INCREMENT_EDITED_VALUE_Pin, GPIO_PIN_SET);
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  ResetI2CBus(); //In case the bus gets stuck during an MCU reset. (works most of the time, not always. TODO: fix)
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  Init16x2LCD();
  HAL_StatusTypeDef status = DS3231_Init(&hi2c1);
  if (status != HAL_OK)
  {
    ClearScreen();
    MoveCursor(1, 1);
    WriteString("Init error (");
    WriteCharacter('0' + status);
    WriteCharacter(')');
    return 1;
  }
  DisplayInfo dispInfo = { 0 };
  dispInfo.tempUnit = TEMP_UNIT_CELSIUS;
  SetDateForDS3231(2026, 1, 29, 4, 23, 30, 55);
  I2C_ErrorHandler(DS3231_SetTimeFormat(0));
  I2C_ErrorHandler(DS3231_SetAlarmTime(23, 31));
  I2C_ErrorHandler(DS3231_ToggleAlarm(1));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  static uint8_t inEditMode = 0;
	  if (inEditMode)
	  {
		  HandleDisplayDuringEditing(&dispInfo);
	  }
	  else
	  {
		  ReadDS3231DataIntoDisplayInfo(&dispInfo);
		  DisplayTime(&dispInfo);

		  uint8_t isAlarmTime = 0;
		  I2C_ErrorHandler(DS3231_IsAlarmTime(&isAlarmTime));
		  if (isAlarmTime)
		  {
			  //Sound the alarm if alarm is enabled, stop it (even mid-alarm) if disabled
			  HAL_GPIO_WritePin(ALARM_SOUND_GPIO_Port, ALARM_SOUND_Pin, dispInfo.alarmEnabled);
		  }
		  else
		  {
			  //Disable the alarm whether or not the alarm is enabled if the alarm time
			  //has passed/did not come yet
			  HAL_GPIO_WritePin(ALARM_SOUND_GPIO_Port, ALARM_SOUND_Pin, GPIO_PIN_RESET);
			  DS3231_SignalAlarmTimePassed();
		  }
	  }

	  if (GetDebouncedButtonState(buttons + PAGE_TOGGLE_BUTTON_INDEX) == BUTTON_STATE_PRESSED)
	  {
		  SignalDisplayToggle();
	  }

	  if (GetDebouncedButtonState(buttons + ALARM_TOGGLE_BUTTON_INDEX) == BUTTON_STATE_PRESSED)
	  {
		  ToggleAlarm();
	  }

	  if (GetDebouncedButtonState(buttons + HOUR_FORMAT_CHANGE_BUTTON_INDEX) == BUTTON_STATE_PRESSED)
	  {
		  ToggleHourFormat();
	  }

	  if (GetDebouncedButtonState(buttons + EDIT_CHOICE_BUTTON_INDEX) == BUTTON_STATE_PRESSED)
	  {
		  if (!inEditMode)
		  {
			  //Get into edit mode
			  inEditMode = 1;
			  StartEditing();
		  }
		  else
		  {
			  uint8_t editingDone = SwitchNextToEdit();
			  if (editingDone)
			  {
				  inEditMode = 0;
				  EndEditing();
				  WriteDispInfoDataIntoDS3231(&dispInfo);
			  }
		  }
	  }

	  if (GetDebouncedButtonState(buttons + INCREMENT_EDITED_VALUE_BUTTON_INDEX) == BUTTON_STATE_PRESSED)
	  {
		  if (inEditMode)
		  {
			  uint16_t yearBeforeEdit = dispInfo.year;
			  IncrementCurrentlyEditedValue(&dispInfo);
			  uint16_t yearAfterEdit = dispInfo.year;
			  uint8_t highestTwoDigitsBeforeEdit = (yearBeforeEdit - (yearBeforeEdit % 100)) / 100;
			  uint8_t highestTwoDigitsAfterEdit = (yearAfterEdit - (yearAfterEdit % 100)) / 100;
			  if (highestTwoDigitsBeforeEdit != highestTwoDigitsAfterEdit)
			  {
				  //User forwarded time more than a century.
				  currentCentury++;
			  }
		  }
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OnboardLED_GPIO_Port, OnboardLED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Pin_RS_Pin|Pin_RW_Pin|Pin_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ALARM_SOUND_GPIO_Port, ALARM_SOUND_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : OnboardLED_Pin */
  GPIO_InitStruct.Pin = OnboardLED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OnboardLED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PAGE_TOGGLE_Pin ALARM_TOGGLE_Pin HOUR_FORMAT_CHANGE_Pin EDIT_CHOICE_Pin
                           INCREMENT_EDITED_VALUE_Pin */
  GPIO_InitStruct.Pin = PAGE_TOGGLE_Pin|ALARM_TOGGLE_Pin|HOUR_FORMAT_CHANGE_Pin|EDIT_CHOICE_Pin
                          |INCREMENT_EDITED_VALUE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Pin_RS_Pin Pin_RW_Pin Pin_EN_Pin */
  GPIO_InitStruct.Pin = Pin_RS_Pin|Pin_RW_Pin|Pin_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ALARM_SOUND_Pin */
  GPIO_InitStruct.Pin = ALARM_SOUND_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ALARM_SOUND_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Pin_D0_Pin Pin_D1_Pin Pin_D2_Pin Pin_D3_Pin */
  GPIO_InitStruct.Pin = Pin_D0_Pin|Pin_D1_Pin|Pin_D2_Pin|Pin_D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Pin_D4_Pin Pin_D5_Pin Pin_D6_Pin Pin_D7_Pin */
  GPIO_InitStruct.Pin = Pin_D4_Pin|Pin_D5_Pin|Pin_D6_Pin|Pin_D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
