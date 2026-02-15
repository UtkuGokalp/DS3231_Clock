/*
 * editor.c
 *
 *  Created on: Feb 8, 2026
 *      Author: ugklp
 */

#include "editor.h"

typedef enum CURRENTLY_EDITING
{
	CURRENTLY_EDITING_HOURS = 0,
	CURRENTLY_EDITING_MINUTES,
	CURRENTLY_EDITING_SECONDS,
	CURRENTLY_EDITING_DAY_OF_MONTH,
	CURRENTLY_EDITING_MONTH,
	CURRENTLY_EDITING_YEAR,
	CURRENTLY_EDITING_DAY_OF_WEEK,
	CURRENTLY_EDITING_ALARM_HOURS,
	CURRENTLY_EDITING_ALARM_MINUTES,
	CURRENTLY_EDITING_COUNT,
} CURRENTLY_EDITING;

static CURRENTLY_EDITING currentlyEditedValue = CURRENTLY_EDITING_HOURS;

//Makes sure a value changes only in a given range. If the value exceeds the boundaries,
//the opposite boundary is returned.
static uint8_t ClampWrapped(uint8_t value, uint8_t min, uint8_t max)
{
	if (max < min)
	{
		max = min;
	}

	if (value > max)
	{
		return min;
	}
	else if (value < min)
	{
		return max;
	}
	return value;
}

void HandleDisplayDuringEditing(const DisplayInfo* info)
{
	switch (currentlyEditedValue)
	{
	case CURRENTLY_EDITING_HOURS:
		MoveCursor(1, 2);
		break;
	case CURRENTLY_EDITING_MINUTES:
		MoveCursor(1, 5);
		break;
	case CURRENTLY_EDITING_SECONDS:
		MoveCursor(1, 8);
		break;
	case CURRENTLY_EDITING_DAY_OF_MONTH:
		MoveCursor(2, 2);
		break;
	case CURRENTLY_EDITING_MONTH:
		MoveCursor(2, 6);
		break;
	case CURRENTLY_EDITING_YEAR:
		MoveCursor(2, 11);
		break;
	case CURRENTLY_EDITING_DAY_OF_WEEK:
		MoveCursor(2, 16);
		break;
	case CURRENTLY_EDITING_ALARM_HOURS:
		MoveCursor(1, 23);
		break;
	case CURRENTLY_EDITING_ALARM_MINUTES:
		MoveCursor(1, 26);
		break;
	default:
		//Don't do anything.
		break;
	}
}

void IncrementCurrentlyEditedValue(DisplayInfo* info)
{
	switch (currentlyEditedValue)
	{
	case CURRENTLY_EDITING_HOURS:
		if (info->displayFormat == DISPLAY_FORMAT_12H)
		{
			info->hours = ClampWrapped(info->hours + 1, 1, 12);
		}
		else
		{
			info->hours = ClampWrapped(info->hours + 1, 0, 23);
		}
		break;
	case CURRENTLY_EDITING_MINUTES:
		info->minutes = ClampWrapped(info->minutes + 1, 0, 59);
		break;
	case CURRENTLY_EDITING_SECONDS:
		info->seconds = ClampWrapped(info->seconds + 1, 0, 59);
		break;
	case CURRENTLY_EDITING_DAY_OF_MONTH:
		info->dayOfTheMonth = ClampWrapped(info->dayOfTheMonth + 1, 1, 31);
		break;
	case CURRENTLY_EDITING_MONTH:
		info->month = ClampWrapped(info->month + 1, 1, 12);
		break;
	case CURRENTLY_EDITING_YEAR:
		//year is 16 bit, don't use Clamp() as it works on uint8_t.
		//Sure I could make it take in uint16_t too but oh well, didn't feel like that was necessary.
		info->year = (info->year + 1) % 10000; //Max allowed year is 9999
		break;
	case CURRENTLY_EDITING_DAY_OF_WEEK:
		info->dayOfTheWeek = ClampWrapped(info->dayOfTheWeek + 1, 1, 7);
		break;
	case CURRENTLY_EDITING_ALARM_HOURS:
		if (info->displayFormat == DISPLAY_FORMAT_12H)
		{
			info->alarmHours = ClampWrapped(info->alarmHours + 1, 1, 12);
		}
		else
		{
			info->alarmHours = ClampWrapped(info->alarmHours + 1, 0, 23);
		}
		break;
	case CURRENTLY_EDITING_ALARM_MINUTES:
		info->alarmMinutes = ClampWrapped(info->alarmMinutes + 1, 0, 59);
		break;
	default:
		//Don't do anything.
		break;
	}
	//Update the display here because if the display is updated in the display handling method, the cursor
	//gets messed up for some reason, whether this function is called before or after the switch statement.
	//Updating the display here essentially means the display is updated only once when necessary and then the
	//display handling function takes control back again.
	DisplayTime(info);
}

void StartEditing(void)
{
	currentlyEditedValue = CURRENTLY_EDITING_HOURS;
	DisplayAndCursorControl(1, 0, 1);
}

void EndEditing(void)
{
	DisplayAndCursorControl(1, 0, 0);
}

uint8_t SwitchNextToEdit(void)
{
	currentlyEditedValue = (currentlyEditedValue + 1) % CURRENTLY_EDITING_COUNT;
	//If the value is 0 after the previous expression, that means the edited values available are done
	//and we wrapped back to the beginning.
	return currentlyEditedValue == 0;
}
