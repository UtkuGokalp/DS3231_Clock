/*
 * ds3231.h
 *
 *  Created on: Jan 27, 2026
 *      Author: ugklp
 */

#ifndef INC_DS3231_H_
#define INC_DS3231_H_

#include "stm32f1xx_hal.h"

#define DS3231_DEV_ADDR 										0b1101000
#define DS3231_REG_ADDR_SECONDS 								0x00
#define DS3231_REG_ADDR_MINUTES 								0x01
#define DS3231_REG_ADDR_HOURS	 								0x02
#define DS3231_REG_ADDR_DAY_OF_WEEK								0x03
#define DS3231_REG_ADDR_DAY_OF_MONTH	 						0x04
#define DS3231_REG_ADDR_MONTH_AND_CENTURY 						0x05
#define DS3231_REG_ADDR_YEAR			 						0x06
#define DS3231_REG_ADDR_ALARM1_SECONDS	 						0x07
#define DS3231_REG_ADDR_ALARM1_MINS		 						0x08
#define DS3231_REG_ADDR_ALARM1_HOURS	 						0x09
#define DS3231_REG_ADDR_ALARM1_DAY_OF_WEEK_AND_MONTH		 	0x0A
#define DS3231_REG_ADDR_ALARM2_MINS		 						0x0B
#define DS3231_REG_ADDR_ALARM2_HOURS	 						0x0C
#define DS3231_REG_ADDR_ALARM2_DAY_OF_WEEK_AND_MONTH			0x0D
#define DS3231_REG_ADDR_CONTROL			 						0x0E
#define DS3231_REG_ADDR_STATUS			 						0x0F
#define DS3231_REG_ADDR_AGING_OFFSET	 						0x10
#define DS3231_REG_ADDR_TEMP_MSB	 							0x11
#define DS3231_REG_ADDR_TEMP_LSB	 							0x12

//Initializes the DS3231 chip and the internal workings of the software as well.
HAL_StatusTypeDef DS3231_Init(I2C_HandleTypeDef* handle);

/*
  Writes the data in the given buffer to the specified register of the DS3231.
  Supports auto-increments, meaning you can write to consecutive registers with one call.
*/
HAL_StatusTypeDef DS3231_WriteToRegister(uint16_t registerAddress, uint8_t* buffer, uint16_t bufferSize);

/*
 Reads data from the specified register of the DS3231 into the given buffer.
 Supports auto-increments, meaning you can write to consecutive registers with one call.
*/
HAL_StatusTypeDef DS3231_ReadFromRegister(uint16_t registerAddress, uint8_t* buffer, uint16_t bufferSize);

/*
 Sets the time format of DS3231 to either 12h or 24h format.
*/
HAL_StatusTypeDef DS3231_SetTimeFormat(uint8_t is12hrFormat);

//Returns the value in the seconds register.
HAL_StatusTypeDef DS3231_ReadSeconds(uint8_t* result);

//Writes the given value of seconds to the DS3231's seconds register. Range is 00-59.
HAL_StatusTypeDef DS3231_WriteSeconds(uint8_t value);

//Returns the value in the minutes register.
HAL_StatusTypeDef DS3231_ReadMinutes(uint8_t* result);

//Writes the given value of minutes to DS3231's minutes register. Range is 00-59.
HAL_StatusTypeDef DS3231_WriteMinutes(uint8_t value);

//Returns the value in the hours register.
//Any one of the pointers can be passed NULL if the caller doesn't care about that value.
HAL_StatusTypeDef DS3231_ReadHours(uint8_t* result, uint8_t* is12HrMode, uint8_t* isPM);

//Writes the given value of hours to DS3231's minutes register. Range is 00-23. AM/PM calculation is done automatically if time format is 12h.
HAL_StatusTypeDef DS3231_WriteHours(uint8_t value);

//Reads whether DS3231 is working in 12h or 24h format. 1 indicates 12h format, 0 indicates 24h format.
HAL_StatusTypeDef DS3231_Is12hrFormatEnabled(uint8_t* result);

/*
  Reads whether the current time is in AM or PM. The result is only
  meaningful if DS3231 is operating in 12h format. 1 means PM, 0 means AM.
*/
HAL_StatusTypeDef DS3231_IsTimePM(uint8_t* result);

//Reads days of the week infomation. Result is between 1 and 7.
HAL_StatusTypeDef DS3231_ReadDayOfTheWeek(uint8_t* result);

//Writes the given day into DS3231's relevant register. Value's range is 1-7.
HAL_StatusTypeDef DS3231_WriteDayOfTheWeek(uint8_t value);

//Reads the day of the month information. Result is between 1 and 31.
HAL_StatusTypeDef DS3231_ReadDayOfTheMonth(uint8_t* result);

//Writes the given day into DS3231's relevant register. Value's range is 1-31.
HAL_StatusTypeDef DS3231_WriteDayOfTheMonth(uint8_t value);

//Reads the month information. Result is between 1 and 12.
HAL_StatusTypeDef DS3231_ReadMonth(uint8_t* result);

//Writes the month to DS3231. Value is between 1 and 12.
HAL_StatusTypeDef DS3231_WriteMonth(uint8_t value);

//Reads the century bit currently stored in the register. Result is 0 or 1.
HAL_StatusTypeDef DS3231_ReadCenturyBit(uint8_t* result);

//Sets the century bit of the month register to the given value. If value is not 0, it is interpreted as 1.
HAL_StatusTypeDef DS3231_WriteCenturyBit(uint8_t value);

//Reads the year value from DS3231. The range is 00-99 (inclusive).
HAL_StatusTypeDef DS3231_ReadYear(uint8_t* result);

//Writes the year value into DS3231. The range is 00-99 (inclusive).
HAL_StatusTypeDef DS3231_WriteYear(uint8_t value);

//Reads the control register from DS3231.
HAL_StatusTypeDef DS3231_ReadControlRegister(uint8_t* result);

//Writes into the control register of DS3231.
HAL_StatusTypeDef DS3231_WriteToControlRegister(uint8_t value);

//Reads the status register from DS3231.
HAL_StatusTypeDef DS3231_ReadStatusRegister(uint8_t* result);

//Writes into the status register of DS3231.
HAL_StatusTypeDef DS3231_WriteToStatusRegister(uint8_t value);

//Reads the alarm status from DS3231. Value = 0 means disabled, value = 1 means enabled.
HAL_StatusTypeDef DS3231_IsAlarmEnabled(uint8_t* result);

//Toggles the alarm of DS3231. Value = 0 means disabled, value > 0 means enabled.
HAL_StatusTypeDef DS3231_ToggleAlarm(uint8_t value);

/*
  Reads the currently set alarm time from DS3231.
  0 <= minutes <= 59
  0 <= hours <= 23 for 24h format
  0 <= hours <= 12 + AM/PM information for 12h format
  Any one of the pointers can be passed NULL if the caller doesn't care about that result.
*/
HAL_StatusTypeDef DS3231_ReadAlarmTime(uint8_t* hours, uint8_t* minutes, uint8_t* is12hFormat, uint8_t* isPM);

//Sets an alarm time. Doesn't enable/disable the alarm. 0 <= hours <= 23, 0 <= minutes <= 59
HAL_StatusTypeDef DS3231_SetAlarmTime(uint8_t hoursIn24hFormat, uint8_t minutes);

//result = 1 if time has come to sound the alarm. result = 0 otherwise.
HAL_StatusTypeDef DS3231_IsAlarmTime(uint8_t* result);

//Since DS3231 doesn't automatically handle alarm time passing, it needs to be told.
//This is the function that tells DS3231 that the alarm time has passed and that
//we don't need to sound the alarm anymore.
HAL_StatusTypeDef DS3231_SignalAlarmTimePassed(void);

//Reads the temperature bits from DS3231 and returns it without any processing whatsoever.
HAL_StatusTypeDef DS3231_ReadTemperature(uint16_t* result);

#endif /* INC_DS3231_H_ */
