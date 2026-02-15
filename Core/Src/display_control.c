/*
 * display_control.c
 *
 *  Created on: Jan 27, 2026
 *      Author: ugklp
 */

#include "lcd_HD44780U.h"
#include "display_control.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>

static uint8_t current_page = 1;

static const char* GetDayName(uint8_t dayOfWeek)
{
	if (dayOfWeek < 1)
	{
		dayOfWeek = 1;
	}
	if (dayOfWeek > 7)
	{
		dayOfWeek = 7;
	}
	static const char* days[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
	return days[dayOfWeek - 1];
}

static const char* GetMonthName(uint8_t month)
{
	if (month < 1)
	{
		month = 1;
	}
	if (month > 12)
	{
		month = 12;
	}
	static const char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	return months[month - 1];
}

static float ConvertTemperatureToFloat(uint16_t temp, uint8_t unit)
{
	int8_t integerPortion = (temp & 0xFF00) >> 8;
	uint8_t fractalPortion = (temp & 0x00FF) >> 6; //upper 2 bits of the last 8 bits are used, details are in the datasheet
	float temperature = (float)integerPortion;
	switch (fractalPortion)
	{
	case 0b00:
		temperature += 0.00f;
		break;
	case 0b01:
		temperature += 0.25f;
		break;
	case 0b10:
		temperature += 0.50f;
		break;
	case 0b11:
		temperature += 0.75f;
		break;
	default:
		//Do nothing, we should never hit here.
		break;
	}
	//At this point, temperature is calculated in terms of celsius.
	//Now convert if necessary.
	switch (unit)
	{
	case TEMP_UNIT_CELSIUS:
		//Already celsius, don't do anything.
		break;
	case TEMP_UNIT_FAHRENHEIT:
		temperature *= 1.8f;
		temperature += 32;
		break;
	case TEMP_UNIT_KELVIN:
		temperature += 273.5;
		break;
	default:
		//Unknown unit, simply leave it as celsius.
		break;
	}
	return temperature;
}

void DisplayTime(DisplayInfo* info)
{
	if (info == NULL)
	{
		return;
	}

	//Positions 1-16 are page 1 and 17-32 are page 2.

	info->alarmDisplayFormat = info->displayFormat;

	//PAGE 1
	MoveCursor(1, 1);
	static const uint8_t MAX_CHARS_ON_A_LINE = 17; //16 + 1 to account for the null character since we are using snprintf
	char line[MAX_CHARS_ON_A_LINE];
	snprintf(line, MAX_CHARS_ON_A_LINE, "%02d:%02d:%02d %s    >", info->hours, info->minutes, info->seconds, info->displayFormat == DISPLAY_FORMAT_12H ? (info->isTimePM ? "PM" : "AM") : "  ");
	WriteString(line);
	MoveCursor(2, 1);

	snprintf(line, MAX_CHARS_ON_A_LINE, "%02d %s %04d  %s", info->dayOfTheMonth, GetMonthName(info->month), info->year, GetDayName(info->dayOfTheWeek));
	WriteString(line);

	//PAGE 2
	MoveCursor(1, 17);
	snprintf(line, MAX_CHARS_ON_A_LINE, "<%s%02d:%02d%s", info->alarmDisplayFormat == DISPLAY_FORMAT_12H ? "   " : "    ",
														  info->alarmHours,
														  info->alarmMinutes,
														  info->alarmDisplayFormat == DISPLAY_FORMAT_12H ? ((info->isAlarmTimePM ? " PM" : " AM")) : "   ");

	WriteString(line);
	if (info->alarmEnabled == ALARM_ENABLED)
	{
		MoveCursor(1, 32);
		SendByte(0xA5); //A dot character for this particular LCD.
	}
	else
	{
		MoveCursor(1, 32);
		WriteCharacter(' '); //Clear the dot if the alarm is not enabled.
	}

	MoveCursor(2, 17);
	float temperature = ConvertTemperatureToFloat(info->temperature, info->tempUnit);

	//The space at the end of the string is intentional, it's not a typo
	snprintf(line, MAX_CHARS_ON_A_LINE, "   %s%02d.%02d ",
			temperature < 0 ? "" : "+",
			(int)temperature,
			(int)((temperature - (int)temperature) * 100));
	WriteString(line);

	switch (info->tempUnit)
	{
	case TEMP_UNIT_CELSIUS:
		WriteString("Cel");
		break;
	case TEMP_UNIT_FAHRENHEIT:
		WriteString("Fah");
		break;
	case TEMP_UNIT_KELVIN:
		WriteString("Kel");
		break;
	default:
		//If the value isn't specified don't display any unit
		break;
	}
}

void SwitchToPage(uint8_t page)
{
	if (page < 1)
	{
		page = 1;
	}
	else if (page > 2)
	{
		page = 2;
	}

	if (page == current_page)
	{
		return;
	}

	if (page == 1) //Currently on page 2, switch to page 1
	{
		ShiftDisplayRight(16);
	}
	else if (page == 2) //Currently on page 1, switch to page 2
	{
		ShiftDisplayLeft(16);
	}

	current_page = page;
}

void ToggleDisplayedPage(void)
{
	SwitchToPage(current_page == 1 ? 2 : 1);
}

uint8_t SignalDisplayToggle(void)
{
	static uint32_t lastToggleTime = 0;
	uint32_t currentTime = HAL_GetTick();
	uint32_t elapsedTime = currentTime - lastToggleTime;
	if (elapsedTime >= DISPLAY_TOGGLE_COOLDOWN_TIME_MS)
	{
		ToggleDisplayedPage();
		lastToggleTime = currentTime;
		return DISPLAY_TOGGLE_ACCEPTED;
	}
	return DISPLAY_TOGGLE_REJECTED;
}

uint8_t GetCurrentPage(void)
{
	return current_page;
}
