/*
 * ds3231.c
 *
 *  Created on: Jan 27, 2026
 *      Author: ugklp
 */

#include "utils.h"
#include "ds3231.h"
#include "stm32f1xx_hal.h"

static I2C_HandleTypeDef* i2cHandle;

//Returns the 12h format equivalent of the given 24h time.
//Sets isPM to 1 if time is PM. Sets it to 0 if it is AM.
static uint8_t ConvertFrom24hTo12hFormat(uint8_t timeIn24h, uint8_t* isPM)
{
	if (timeIn24h == 0)
	{
		if (isPM != NULL)
		{
			*isPM = 0;
		}
		return 12;
	}
	else if (timeIn24h < 12)
	{
		if (isPM != NULL)
		{
			*isPM = 0;
		}
		return timeIn24h;
	}
	else if (timeIn24h == 12)
	{
		if (isPM != NULL)
		{
			*isPM = 1;
		}
		return timeIn24h;
	}
	else
	{
		if (isPM != NULL)
		{
			*isPM = 1;
		}
		return timeIn24h - 12;
	}
}

HAL_StatusTypeDef DS3231_Init(I2C_HandleTypeDef* handle)
{
	i2cHandle = handle;

	/*
	  The only alarm mode we will be using is Alarm 2 in HH:MM match mode. For this mode, the following
	  needs to be set (see datasheet for more info):
	  A2M2 = 0
	  A2M3 = 0
	  A2M4 = 1

	  A2M2 and A2M3 will always be set to zero when setting the alarm time due to the fact that
	  0 <= minutes <= 59 and 0 <= hours <= 23 . This fact means the top-most bit (A2M2 and A2M3
	  respectively) will always be zeros.
	  A2M4 on the other hand needs to be set to 1. However since we don't have anything to set in
	  that register, we can simply set that bit once in the init function and forget about that
	  register entirely. The following code sets A2M4 bit.
	*/
	uint8_t buffer = 0x80;
	return DS3231_WriteToRegister(DS3231_REG_ADDR_ALARM2_DAY_OF_WEEK_AND_MONTH, &buffer, 1);
}

HAL_StatusTypeDef DS3231_WriteToRegister(uint16_t registerAddress, uint8_t* buffer, uint16_t bufferSize)
{
	if (i2cHandle == NULL || buffer == NULL)
	{
		return HAL_ERROR;
	}
	return HAL_I2C_Mem_Write(i2cHandle, DS3231_DEV_ADDR << 1, registerAddress, I2C_MEMADD_SIZE_8BIT, buffer, bufferSize, HAL_MAX_DELAY);
}

HAL_StatusTypeDef DS3231_ReadFromRegister(uint16_t registerAddress, uint8_t* buffer, uint16_t bufferSize)
{
	if (i2cHandle == NULL || buffer == NULL)
	{
		return HAL_ERROR;
	}
	return HAL_I2C_Mem_Read(i2cHandle, DS3231_DEV_ADDR << 1, registerAddress, I2C_MEMADD_SIZE_8BIT, buffer, bufferSize, HAL_MAX_DELAY);
}

HAL_StatusTypeDef DS3231_SetTimeFormat(uint8_t is12hrFormat)
{
	uint8_t currentRegisterValue = 0;
	HAL_StatusTypeDef status = DS3231_ReadFromRegister(DS3231_REG_ADDR_HOURS, &currentRegisterValue, 1);
	if (status != HAL_OK)
	{
		return status;
	}

	if (is12hrFormat) //needs a logic 1 at bit 6
	{
		currentRegisterValue |= (1 << 6);
	}
	else //needs a logic 0 at bit 6
	{
		currentRegisterValue &= ~(1 << 6);
	}

	return DS3231_WriteToRegister(DS3231_REG_ADDR_HOURS, &currentRegisterValue, 1);
}

HAL_StatusTypeDef DS3231_ReadSeconds(uint8_t* result)
{
	uint8_t buf = 0;
	HAL_StatusTypeDef status = DS3231_ReadFromRegister(DS3231_REG_ADDR_SECONDS, &buf, 1);
	*result = BCDToBinary(buf);
	return status;
}

HAL_StatusTypeDef DS3231_WriteSeconds(uint8_t value)
{
	if (value > 59)
	{
		value = 59;
	}
	value = value & 0x7F; //Clear the MSB
	value = BinaryToBCD(value);
	return DS3231_WriteToRegister(DS3231_REG_ADDR_SECONDS, &value, 1);
}

HAL_StatusTypeDef DS3231_ReadMinutes(uint8_t* result)
{
	uint8_t buf = 0;
	HAL_StatusTypeDef status = DS3231_ReadFromRegister(DS3231_REG_ADDR_MINUTES, &buf, 1);
	*result = BCDToBinary(buf);
	return status;
}

HAL_StatusTypeDef DS3231_WriteMinutes(uint8_t value)
{
	if (value > 59)
	{
		value = 59;
	}
	value = value & 0x7F; //Clear the MSB
	value = BinaryToBCD(value);
	return DS3231_WriteToRegister(DS3231_REG_ADDR_MINUTES, &value, 1);
}

HAL_StatusTypeDef DS3231_ReadHours(uint8_t* result, uint8_t* is12HrMode, uint8_t* isPM)
{
	uint8_t buf = 0;
	HAL_StatusTypeDef status = DS3231_ReadFromRegister(DS3231_REG_ADDR_HOURS, &buf, 1);

	uint8_t bitmask = 0x40;
	if (buf & bitmask) //If the 6th bit is set, we are in 12 hour mode
	{
		if (is12HrMode)
		{
			*is12HrMode = 1;
		}
		//bit 5 indicates AM (logic 0) or PM (logic 1)
		if (isPM)
		{
			*isPM = (buf & 0x30) >> 5;
		}
		//Clear the AM/PM bit
		buf &= 0x1F;
	}
	else //Otherwise we are in 24 hour mode
	{
		if (is12HrMode)
		{
			*is12HrMode = 0;
		}
		//bit 5 is part of BCD formatting
		buf &= 0x3F;
	}

	if (result)
	{
		*result = BCDToBinary(buf);
	}
	return status;
}

HAL_StatusTypeDef DS3231_WriteHours(uint8_t value)
{
	if (value > 23)
	{
		value = 23;
	}

	uint8_t is12HrFormat = 0;
	DS3231_Is12hrFormatEnabled(&is12HrFormat);

	uint8_t buffer = 0;
	if (is12HrFormat) // value must be between 1 and 12
	{
		uint8_t isPM = 0;
		value = ConvertFrom24hTo12hFormat(value, &isPM);

		//MSB (bit 7) should be 0. Bit 6 should be 1 for 12h format. Bit 5 indicates AM(0)/PM(1).
		//Rest is time in BCD.
		buffer |= 1 << 6;
		buffer |= isPM << 5;
		buffer |= BinaryToBCD(value);
	}
	else //24 hour format
	{
		//MSB (bit 7) should be 0. Bit 6 should be 0 for 24h format. The rest is time in BCD.
		buffer |= BinaryToBCD(value);
	}
	return DS3231_WriteToRegister(DS3231_REG_ADDR_HOURS, &buffer, 1);
}

HAL_StatusTypeDef DS3231_Is12hrFormatEnabled(uint8_t* result)
{
	uint8_t buf = 0;
	HAL_StatusTypeDef status = DS3231_ReadFromRegister(DS3231_REG_ADDR_HOURS, &buf, 1);
	*result = (buf & 0x40) >> 6;
	return status;
}

HAL_StatusTypeDef DS3231_IsTimePM(uint8_t* result)
{
	uint8_t buf = 0;
	HAL_StatusTypeDef status = DS3231_ReadFromRegister(DS3231_REG_ADDR_HOURS, &buf, 1);
	*result = (buf & 0x20) >> 5;
	return status;
}

HAL_StatusTypeDef DS3231_ReadDayOfTheWeek(uint8_t* result)
{
	//No need to convert this BCD value into binary. It's already is between 1-7.
	//Values in that range have the same representation in both binary and BCD.
	return DS3231_ReadFromRegister(DS3231_REG_ADDR_DAY_OF_WEEK, result, 1);
}

HAL_StatusTypeDef DS3231_WriteDayOfTheWeek(uint8_t value)
{
	if (value < 1)
	{
		value = 1;
	}
	else if (value > 7)
	{
		value = 7;
	}
	//No need to convert this to BCD. It's already is between 1-7.
	//Values in that range have the same representation in both binary and BCD.
	return DS3231_WriteToRegister(DS3231_REG_ADDR_DAY_OF_WEEK, &value, 1);
}

HAL_StatusTypeDef DS3231_ReadDayOfTheMonth(uint8_t* result)
{
	uint8_t buffer = 0;
	HAL_StatusTypeDef status = DS3231_ReadFromRegister(DS3231_REG_ADDR_DAY_OF_MONTH, &buffer, 1);
	uint8_t bitmask = 0x3F;
	*result = BCDToBinary(buffer & bitmask);
	return status;
}

HAL_StatusTypeDef DS3231_WriteDayOfTheMonth(uint8_t value)
{
	if (value < 1)
	{
		value = 1;
	}
	else if (value > 31)
	{
		value = 31;
	}
	value = BinaryToBCD(value);
	return DS3231_WriteToRegister(DS3231_REG_ADDR_DAY_OF_MONTH, &value, 1);
}

HAL_StatusTypeDef DS3231_ReadMonth(uint8_t* result)
{
	uint8_t buffer = 0;
	HAL_StatusTypeDef status = DS3231_ReadFromRegister(DS3231_REG_ADDR_MONTH_AND_CENTURY, &buffer, 1);
	uint8_t bitmask = 0x1F;
	*result = BCDToBinary(buffer & bitmask);
	return status;
}

HAL_StatusTypeDef DS3231_WriteMonth(uint8_t value)
{
	if (value < 1)
	{
		value = 1;
	}
	else if (value > 12)
	{
		value = 12;
	}
	uint8_t century = 0;
	HAL_StatusTypeDef status = DS3231_ReadCenturyBit(&century);
	if (status != HAL_OK)
	{
		return status;
	}
	value = (century << 7) | BinaryToBCD(value);
	return DS3231_WriteToRegister(DS3231_REG_ADDR_MONTH_AND_CENTURY, &value, 1);
}

HAL_StatusTypeDef DS3231_ReadCenturyBit(uint8_t* result)
{
	uint8_t buffer = 0;
	HAL_StatusTypeDef status = DS3231_ReadFromRegister(DS3231_REG_ADDR_MONTH_AND_CENTURY, &buffer, 1);
	uint8_t bitmask = 0x80;
	*result = BCDToBinary(buffer & bitmask);
	return status;
}

HAL_StatusTypeDef DS3231_WriteCenturyBit(uint8_t value)
{
	if (value != 0)
	{
		value = 1;
	}
	//At this point, value is either 0 or 1.

	uint8_t buffer = 0;
	HAL_StatusTypeDef status = DS3231_ReadFromRegister(DS3231_REG_ADDR_MONTH_AND_CENTURY, &buffer, 1); //Read the current register

	if (status != HAL_OK)
	{
		return status;
	}

	buffer &= 0x7F; //Clear the MSB
	buffer |= (value << 7); //Put value into MSB

	status = DS3231_WriteToRegister(DS3231_REG_ADDR_MONTH_AND_CENTURY, &buffer, 1); //Write the new bits to the register
	return status;
}

HAL_StatusTypeDef DS3231_ReadYear(uint8_t* result)
{
	uint8_t buffer = 0;
	HAL_StatusTypeDef status = DS3231_ReadFromRegister(DS3231_REG_ADDR_YEAR, &buffer, 1);
	*result = BCDToBinary(buffer);
	return status;
}

HAL_StatusTypeDef DS3231_WriteYear(uint8_t value)
{
	if (value > 99)
	{
		value = 99;
	}
	value = BinaryToBCD(value);
	return DS3231_WriteToRegister(DS3231_REG_ADDR_YEAR, &value, 1);
}

HAL_StatusTypeDef DS3231_ReadControlRegister(uint8_t* result)
{
	return DS3231_ReadFromRegister(DS3231_REG_ADDR_CONTROL, result, 1);
}

HAL_StatusTypeDef DS3231_WriteToControlRegister(uint8_t value)
{
	return DS3231_WriteToRegister(DS3231_REG_ADDR_CONTROL, &value, 1);
}

HAL_StatusTypeDef DS3231_ReadStatusRegister(uint8_t* result)
{
	return DS3231_ReadFromRegister(DS3231_REG_ADDR_STATUS, result, 1);
}

HAL_StatusTypeDef DS3231_WriteToStatusRegister(uint8_t value)
{
	return DS3231_WriteToRegister(DS3231_REG_ADDR_STATUS, &value, 1);
}

HAL_StatusTypeDef DS3231_IsAlarmEnabled(uint8_t* result)
{
	uint8_t buffer = 0;
	HAL_StatusTypeDef status = DS3231_ReadControlRegister(&buffer);
	*result = (buffer & 0x02) >> 1; //Alarm 2's bit is 2nd LSB.
	return status;
}

HAL_StatusTypeDef DS3231_ToggleAlarm(uint8_t value)
{
	if (value != 0)
	{
		value = 1;
	}

	uint8_t buffer = 0;
	HAL_StatusTypeDef status = DS3231_ReadControlRegister(&buffer);
	if (status != HAL_OK)
	{
		return status;
	}

	//The alarm we wanna use to detect HH:MM match is alarm 2. We can ignore
	//the existence of alarm 1 on the chip since that one detects seconds as well.
	///Alarm 2 enabled bit is the 2nd LSB
	uint8_t clearMask = ~0x02;
	buffer &= clearMask; //Clear the 2nd LSB
	buffer |= (value << 1); //Set value into 2nd LSB
	return DS3231_WriteToControlRegister(buffer);
}

HAL_StatusTypeDef DS3231_ReadAlarmTime(uint8_t* hours, uint8_t* minutes, uint8_t* is12hFormat, uint8_t* isPM)
{
	uint8_t buffer = 0;
	HAL_StatusTypeDef status = DS3231_ReadFromRegister(DS3231_REG_ADDR_ALARM2_MINS, &buffer, 1);
	if (status != HAL_OK)
	{
		return status;
	}

	//MSB should be zero anyways but just in case, clear it since it doesn't mean anything for minutes.
	buffer &= ~(1 << 7);
	if (minutes)
	{
		*minutes = BCDToBinary(buffer);
	}

	status = DS3231_ReadFromRegister(DS3231_REG_ADDR_ALARM2_HOURS, &buffer, 1);
	if (status != HAL_OK)
	{
		return status;
	}

	uint8_t is12h = (buffer >> 6) & 0x01;
	if (is12h)
	{
		if (is12hFormat)
		{
			*is12hFormat = 1;
		}
		if (isPM)
		{
			*isPM = (buffer >> 5) & 0x01;
		}
		if (hours)
		{
			*hours = BCDToBinary(buffer & 0x1F);
		}
	}
	else
	{
		if (is12hFormat)
		{
			*is12hFormat = 0;
		}
		if (hours)
		{
			*hours = BCDToBinary(buffer & 0x3F);
		}
	}

	return status;
}

HAL_StatusTypeDef DS3231_SetAlarmTime(uint8_t hoursIn24hFormat, uint8_t minutes)
{
	if (hoursIn24hFormat > 23)
	{
		hoursIn24hFormat = 23;
	}
	if (minutes > 59)
	{
		minutes = 59;
	}

	minutes = BinaryToBCD(minutes);
	//MSB needs to be set to zero for correct alarm detection. This is ensured
	//by the range of minutes but do it here explicitly just to be safe.
	minutes &= ~(1 << 7);
	HAL_StatusTypeDef status = DS3231_WriteToRegister(DS3231_REG_ADDR_ALARM2_MINS, &minutes, 1);
	if (status != HAL_OK)
	{
		return status;
	}

	uint8_t formatIs12Hr = 0;
	status = DS3231_Is12hrFormatEnabled(&formatIs12Hr);
	if (status != HAL_OK)
	{
		return status;
	}

	if (formatIs12Hr)
	{
		uint8_t isPM = 0;
		uint8_t hoursIn12hFormat = ConvertFrom24hTo12hFormat(hoursIn24hFormat, &isPM);
		hoursIn12hFormat = BinaryToBCD(hoursIn12hFormat);
		hoursIn12hFormat &= ~(1 << 7); //MSB needs to be set to 0.
		hoursIn12hFormat |= 1 << 6; //2nd MSB needs to be set to 1 for 12 hour format.
		hoursIn12hFormat |= isPM << 5; //Bit 5 needs to be set to 1 for PM, 0 for AM.
		return DS3231_WriteToRegister(DS3231_REG_ADDR_ALARM2_HOURS, &hoursIn12hFormat, 1);
	}
	else //24 hour format
	{
		hoursIn24hFormat = BinaryToBCD(hoursIn24hFormat);
		//MSB needs to be set to zero for correct alarm detection.
		//2nd MSB needs to be set to zero to indicate 24h time format.
		hoursIn24hFormat &= 0x3F;
		return DS3231_WriteToRegister(DS3231_REG_ADDR_ALARM2_HOURS, &hoursIn24hFormat, 1);
	}
}

HAL_StatusTypeDef DS3231_IsAlarmTime(uint8_t* result)
{
	uint8_t clockHours = 0, clockMinutes = 0, isClock12hrMode = 0, isClockPM = 0;
	uint8_t alarmHours = 0, alarmMinutes = 0, isAlarm12hrMode = 0, isAlarmPM = 0;
	HAL_StatusTypeDef status = DS3231_ReadHours(&clockHours, &isClock12hrMode, &isClockPM);
	if (status != HAL_OK)
	{
		return status;
	}

	status = DS3231_ReadMinutes(&clockMinutes);
	if (status != HAL_OK)
	{
		return status;
	}

	status = DS3231_ReadAlarmTime(&alarmHours, &alarmMinutes, &isAlarm12hrMode, &isAlarmPM);
	if (status != HAL_OK)
	{
		return status;
	}

	//Ensure both of the times are calculated as 12 hour modes
	//(just because we already have a function to convert from 24hr to 12hr format)
	if (!isClock12hrMode)
	{
		clockHours = ConvertFrom24hTo12hFormat(clockHours, &isClockPM);
	}

	if (!isAlarm12hrMode)
	{
		alarmHours = ConvertFrom24hTo12hFormat(alarmHours, &isAlarmPM);
	}

	*result = (clockHours == alarmHours) && (clockMinutes == alarmMinutes) && (isClockPM == isAlarmPM);
	return status;
}

HAL_StatusTypeDef DS3231_SignalAlarmTimePassed(void)
{
	uint8_t buffer = 0;
	HAL_StatusTypeDef status = DS3231_ReadStatusRegister(&buffer);
	if (status != HAL_OK)
	{
		return status;
	}

	buffer &= 0x02; //Clear out bit 1 (2nd LSB)
	return DS3231_WriteToStatusRegister(buffer);
}

HAL_StatusTypeDef DS3231_ReadTemperature(uint16_t* result)
{
	uint8_t buffer = 0;
	HAL_StatusTypeDef status = DS3231_ReadFromRegister(DS3231_REG_ADDR_TEMP_MSB, &buffer, 1);
	*result = buffer << 8;

	if (status != HAL_OK)
	{
		return status;
	}

	status = DS3231_ReadFromRegister(DS3231_REG_ADDR_TEMP_LSB, &buffer, 1);
	*result |= buffer;

	return status;
}
