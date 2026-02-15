/*
 * instructions.h
 *
 *	The "chip" referred to in the comments of this file is the HD44780U chip that is used for controlling a generic
 *	16x2 LCD screen.
 *  Created on: Jan 16, 2026
 *      Author: ugklp
 */

#ifndef INC_LCD_HD44780U_H_
#define INC_LCD_HD44780U_H_

#include <stdint.h>
#include <stddef.h>

#define arr_size(a)			(sizeof(a) / sizeof((a)[0]))

//Instruction bits correspond to RS-RW-D7-D6-D5-D4-D3-D2-D1-D0 in order. Big endian. Only the lower 10 bits of the instruction are used.
void SendInstruction(uint16_t instruction);

//Initializes a 16x2 LCD screen. Different screens need different initializations, use this method only with 16x2 LCDs.
void Init16x2LCD();

//Clears the entire display
void ClearScreen();

//Sets the cursor to the beginning of the display and returns the display from being shifted to original position.
void ReturnHome();

//Sets cursor move direction and specifies display shift. These operations are performed during data write and read.
//increment = 0 means decrement, != 0 means increment. shift = 0 means no shift, != 0 means shift.
void EntryModeSet(uint8_t increment, uint8_t shiftDisplay);

//Sets entire display on/off, cursor on/off, blinking on/off. 0 means off, not 0 means on for all parameters.
void DisplayAndCursorControl(uint8_t display, uint8_t cursor, uint8_t blink);

//Moves cursor to the right or to the left
void ShiftCursor(uint8_t shiftRight);

//Moves the cursor to the given position on the given line. 1 <= line <= 2 and 1 <= position <= 40.
void MoveCursor(uint8_t line, uint8_t position);

//Returns the line the cursor is currently on. Returns 1 or 2 upon success, another value upon error.
uint8_t GetCurrentLine();

//Shifts display to the right or to the left
void ShiftDisplay(uint8_t shiftRight);

//Shifts the display to the right n times.
void ShiftDisplayRight(size_t n);

//Shifts the display to the left n times.
void ShiftDisplayLeft(size_t n);

//Sets interface data length, number of display lines and character font
void FunctionSet(uint8_t using8Bits, uint8_t using2Lines, uint8_t using5x10Font);

//Sends the given byte to the chip. Where the data will be written to in the chip's memory is determined by the
//address counter on the chip.
void SendByte(uint8_t byte);

//Writes the given character on the screen. The character is selected from chip's internal ROM, which supports
//more than just ASCII characters.
void WriteCharacter(uint8_t character);

//Writes the given string on the screen
void WriteString(const char* text);

//Sets a CGRAM address for the internal address counter of the chip. CGRAM data is sent and received after this
//setting. Only the lowest 6 bits are used.
void SetCGRAMAddress(uint8_t address);

//Sets a DDRAM address for the internal address counter of the chip. DDRAM data is sent and received after this
//setting. Only the lowest 7 bits are used.
void SetDDRAMAddress(uint8_t address);

//Returns whether the chip is busy executing an internal operation. You shouldn't need to explicitly check for this,
//each function provided already checks if the chip is busy before sending the instruction.
uint8_t IsBusy();

//Reads the current address counter value of the chip.
uint8_t ReadAddressCounter();

//Reads from the internal memory of the LCD chip. The address is determined by the chip's internal address counter.
//Either CGRAM Address or DDRAM Address needs to be set before calling this function.
uint8_t ReadByte();

#endif /* INC_LCD_HD44780U_H_ */
