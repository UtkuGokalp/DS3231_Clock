/*
 * display_control.h
 *
 *  Created on: Jan 27, 2026
 *      Author: ugklp
 */

#ifndef INC_DISPLAY_CONTROL_H_
#define INC_DISPLAY_CONTROL_H_

#include <stdint.h>

#define TEMP_UNIT_CELSIUS 0
#define TEMP_UNIT_FAHRENHEIT 1
#define TEMP_UNIT_KELVIN 2

#define ALARM_DISABLED 0
#define ALARM_ENABLED 1

#define DISPLAY_FORMAT_12H 1
#define DISPLAY_FORMAT_24H 0

#define DISPLAY_TOGGLE_COOLDOWN_TIME_MS  500 //in milliseconds

#define DISPLAY_TOGGLE_REJECTED 0
#define DISPLAY_TOGGLE_ACCEPTED 1

typedef struct DisplayInfo
{
	//DISPLAY PAGE 1 INFO
	uint8_t hours; //01-12 or 00-23
	uint8_t minutes; //00-59
	uint8_t seconds; //00-59
	uint8_t displayFormat; //12h (0) or 24h (1)
	uint8_t dayOfTheMonth; //01-31
	uint8_t month; //01-12
	uint16_t year; //00-99 (century information is handled separately)
	uint8_t dayOfTheWeek; //01-07
	uint8_t isTimePM; //1 means PM, 0 means AM. Only used when 12 hour format is active.

	//DISPLAY PAGE 2 INFO
	uint8_t alarmHours; //01-12 or 00-23
	uint8_t alarmMinutes; //00-59
	uint8_t alarmEnabled; //0 for disabled, 1 for enabled
	uint8_t isAlarmTimePM; //1 for PM, 0 for AM
	uint8_t alarmDisplayFormat; //12h (0) or 24h (1)

	//Represented as fixed-point decimal number. Upper 8 bits are the integer part.
	//Upper 2 bits of the lower 8 bits are the fractional part. Other 6 bits are unused.
	//Datasheet explains this more clearly.
	//Make sure the software accounts for the least significant 6 bits being 0 wherever
	//necessary.
	uint16_t temperature;
	uint8_t tempUnit; //celsius (0), fahrenheit (1) or kelvin (2)
} DisplayInfo;

/*
  This function clears whatever was on the LCD display previously, unless info == NULL.
*/
void DisplayTime(DisplayInfo* info);

/*
  1 for display page 1 (time info). 2 for display page 2 (alarm and temperature info).
  Any other value is clamped. No change happens if page is the currently displayed page.
*/
void SwitchToPage(uint8_t page);

/*
  Display page 1 if currently page 2 is displayed and vice versa.
*/
void ToggleDisplayedPage(void);

/*
  Toggles the display if enough time has passed since last toggle signal.
  Doesn't take the time passed since the other display toggle functions called into
  account.
  Returns whether or not the display was toggled. (1 on toggle, 0 on not toggle)
*/
uint8_t SignalDisplayToggle(void);

/*
  Returns the currently displayed page.
*/
uint8_t GetCurrentPage(void);

#endif /* INC_DISPLAY_CONTROL_H_ */
