/*
 * debounced_button.c
 *
 *  Created on: Feb 1, 2026
 *      Author: ugklp
 */

#include <debounced_button.h>
#include "stm32f1xx_hal.h"

DebouncedButton InitButtonWithDefaults(GPIO_TypeDef* gpioChannel,	uint16_t gpioPin, GPIO_PinState expectedSignalWhenPressed)
{
	DebouncedButton button = { 0 };
	button.lastPressedTime = 0;
	button.debounceTime = DEBOUNCED_BUTTON_DEFAULT_DEBOUNCE_TIME_MS;
	button.gpioChannel = gpioChannel;
	button.gpioPin = gpioPin;
	button.expectedSignalWhenPressed = expectedSignalWhenPressed;
	button.lastPressState = !button.expectedSignalWhenPressed; //Accept initial state as not pressed
	button.lastMeasurementTime = 0;
	return button;
}

ButtonState GetDebouncedButtonState(DebouncedButton* button)
{
    uint32_t now = HAL_GetTick();

    uint8_t rawPressed = HAL_GPIO_ReadPin(button->gpioChannel, button->gpioPin) == button->expectedSignalWhenPressed;

    // Button released
    if (!rawPressed)
    {
        button->lastPressState = !button->expectedSignalWhenPressed;
        return BUTTON_STATE_NOT_PRESSED;
    }

    // Debounce check
    if ((now - button->lastPressedTime) < button->debounceTime)
    {
        return BUTTON_STATE_NOT_PRESSED;
    }

    // Rising edge
    if (button->lastPressState != button->expectedSignalWhenPressed)
    {
        button->lastPressState = button->expectedSignalWhenPressed;
        button->lastPressedTime = now;
        return BUTTON_STATE_PRESSED;
    }

    // Held
    const uint8_t BUTTON_HOLD_TIME_MS = 2;
    if ((now - button->lastPressedTime) >= BUTTON_HOLD_TIME_MS)
    {
        return BUTTON_STATE_HELD;
    }

    return BUTTON_STATE_NOT_PRESSED;
}
