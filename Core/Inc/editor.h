/*
 * editor.h
 *
 *  Created on: Feb 8, 2026
 *      Author: ugklp
 */

#ifndef INC_EDITOR_H_
#define INC_EDITOR_H_

#include "display_control.h"
#include "lcd_HD44780U.h"
#include "stdint.h"
#include "stm32f1xx_hal.h"

//Handles the display in editing mode.
void HandleDisplayDuringEditing(const DisplayInfo* info);
//Handles incrementing the values in the editing mode.
void IncrementCurrentlyEditedValue(DisplayInfo* info);
//Starts the editing sequence.
void StartEditing(void);
//Ends the editing.
void EndEditing(void);
//Switches to the next value to edit. Returns 1 if wrapped back to the beginning. Returns 0 otherwise.
uint8_t SwitchNextToEdit(void);

#endif /* INC_EDITOR_H_ */
