/*
 * debounced_button.h
 *
 *  Created on: Feb 1, 2026
 *      Author: ugklp
 */

#ifndef INC_DEBOUNCED_BUTTON_H_
#define INC_DEBOUNCED_BUTTON_H_

#define DEBOUNCED_BUTTON_DEFAULT_DEBOUNCE_TIME_MS		20

#include <stdint.h>
#include "stm32f1xx_hal.h"

#define DEFAULT_BUTTON_STATE 	   BUTTON_STATE_NOT_PRESSED

typedef enum ButtonState
{
	BUTTON_STATE_NOT_PRESSED = 0,
	BUTTON_STATE_PRESSED,
	BUTTON_STATE_HELD,
} ButtonState;

typedef struct DebouncedButton
{
	//These are safe to modify
	uint32_t debounceTime;
	GPIO_TypeDef* gpioChannel;
	uint16_t gpioPin;
	GPIO_PinState expectedSignalWhenPressed;

	//These are handled internally by the functions below.
	//Don't modify unless you know what you are doing.
	uint32_t lastPressedTime;
	GPIO_PinState lastPressState;
	uint32_t lastMeasurementTime;
} DebouncedButton;

//Initializes a button with default values.
DebouncedButton InitButtonWithDefaults(GPIO_TypeDef* gpioChannel,	uint16_t gpioPin, GPIO_PinState expectedSignalWhenPressed);

//Returns if the button is not pressed, was just pressed or is currently held down.
ButtonState GetDebouncedButtonState(DebouncedButton* button);
#endif /* INC_DEBOUNCED_BUTTON_H_ */
