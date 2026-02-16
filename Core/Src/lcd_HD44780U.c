/*
 * instructions.c
 *
 *  Created on: Jan 16, 2026
 *      Author: ugklp
 */

#include <lcd_HD44780U.h>
#include "main.h"
#include <string.h>
#include "stm32f1xx_hal.h"

//All the addresses below are taken from the datasheet
static const uint8_t FIRST_LINE_START_ADDRESS_IN_DDRAM = 0x00;
static const uint8_t FIRST_LINE_END_ADDRESS_IN_DDRAM = 0x27; //0x00 + 40 = 0x27 (both lines are 40 chars long)
static const uint8_t SECOND_LINE_START_ADDRESS_IN_DDRAM = 0x40;
static const uint8_t SECOND_LINE_END_ADDRESS_IN_DDRAM = 0x67; //0x40 + 40 = 0x67 (both lines are 40 chars long)

static void DWT_Init(void)
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; //Enable debug trace
	DWT->CYCCNT = 0; //Reset the cycle counter
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; //Enable the cycle counter
}

static void DWT_delay_us(uint32_t delay)
{
	uint32_t start = DWT->CYCCNT;
	uint32_t cpuCyclesPerMicrosecond = HAL_RCC_GetHCLKFreq() / 1000000;
	uint32_t requiredCycles = delay * cpuCyclesPerMicrosecond;

	//Wait until enough cycles has passed
	while ((DWT->CYCCNT - start) < requiredCycles) { }
}

//For input, pass GPIO_MODE_INPUT. For output, pass GPIO_MODE_OUTPUT_PP.
static void ChangeGPIOPortModes(uint32_t mode)
{
	GPIO_InitTypeDef gpioInit = { 0 };
	gpioInit.Mode = mode;
	gpioInit.Pull = GPIO_NOPULL;
	gpioInit.Speed = GPIO_SPEED_FREQ_LOW;

	//Change GPIOA (D0-D3)
	gpioInit.Pin = Pin_D0_Pin | Pin_D1_Pin | Pin_D2_Pin | Pin_D3_Pin;
	HAL_GPIO_Init(GPIOA, &gpioInit);

	//Change GPIOB (D4-D7)
	gpioInit.Pin = Pin_D4_Pin | Pin_D5_Pin | Pin_D6_Pin | Pin_D7_Pin;
	HAL_GPIO_Init(GPIOB, &gpioInit);
}

//This function expects which read operation needs to be done to already be specified. e.g. if you
//want to check for the busy flag, you need to set RS=low RW=high before calling this function.
static uint8_t ReadLCDMemory_Internal()
{
	//Do NOT wait until busy flag turns off here. In order to read the busy flag, this function needs to
	//be called. If this function checks for busy flag as well, we have infinite recursion and eventual
	//stack overflow.

	ChangeGPIOPortModes(GPIO_MODE_INPUT);

	//Here RS and RW are already set. Before enable pin is used, tAS time needs to pass.
	//In case the caller didn't do it, add the delay here.
	uint32_t tAS = 1; //This is 40 ns minimum. We have a resolution of us, so wait 1 us.
	DWT_delay_us(tAS);

	HAL_GPIO_WritePin(Pin_EN_GPIO_Port, Pin_EN_Pin, GPIO_PIN_SET);
	uint32_t tDDR = 1; //This is 160 ns (max). Since we have 1 us precision, wait for 1 us.
	DWT_delay_us(tDDR); //Wait until data becomes valid.
	//NOTE: The enable signal also needs to stay high for at least PWeh = 230 ns. Since we are
	//waiting 1 us, it includes this delay as well so we don't need to bother with it.
	uint8_t value = 0;
	value |= HAL_GPIO_ReadPin(Pin_D7_GPIO_Port, Pin_D7_Pin) << 7;
	value |= HAL_GPIO_ReadPin(Pin_D6_GPIO_Port, Pin_D6_Pin) << 6;
	value |= HAL_GPIO_ReadPin(Pin_D5_GPIO_Port, Pin_D5_Pin) << 5;
	value |= HAL_GPIO_ReadPin(Pin_D4_GPIO_Port, Pin_D4_Pin) << 4;
	value |= HAL_GPIO_ReadPin(Pin_D3_GPIO_Port, Pin_D3_Pin) << 3;
	value |= HAL_GPIO_ReadPin(Pin_D2_GPIO_Port, Pin_D2_Pin) << 2;
	value |= HAL_GPIO_ReadPin(Pin_D1_GPIO_Port, Pin_D1_Pin) << 1;
	value |= HAL_GPIO_ReadPin(Pin_D0_GPIO_Port, Pin_D0_Pin) << 0;

	HAL_GPIO_WritePin(Pin_EN_GPIO_Port, Pin_EN_Pin, GPIO_PIN_RESET);
	//After enable is set low, the data/address is held for tDHR and tAH respectively. The minimum values of
	//these are 5ns min and 10ns min. Waiting a whole microsecond is very wasteful, so just wait 100 clock
	//cycles with NOP.
	for (int i = 0; i < 100; i++)
	{
		__NOP();
	}
	ChangeGPIOPortModes(GPIO_MODE_OUTPUT_PP);

	return value;
}

void SendInstruction(uint16_t instruction)
{
	while (IsBusy()) { }

	ChangeGPIOPortModes(GPIO_MODE_OUTPUT_PP);
	GPIO_TypeDef* ports[] = { Pin_RS_GPIO_Port, Pin_RW_GPIO_Port, Pin_D7_GPIO_Port, Pin_D6_GPIO_Port,
							  Pin_D5_GPIO_Port, Pin_D4_GPIO_Port, Pin_D3_GPIO_Port, Pin_D2_GPIO_Port,
							  Pin_D1_GPIO_Port, Pin_D0_GPIO_Port };

	uint16_t pins[] 	  = { Pin_RS_Pin, Pin_RW_Pin, Pin_D7_Pin, Pin_D6_Pin, Pin_D5_Pin,
			                  Pin_D4_Pin, Pin_D3_Pin, Pin_D2_Pin, Pin_D1_Pin, Pin_D0_Pin };
	for (int i = 0; i < arr_size(pins); i++)
	{
		HAL_GPIO_WritePin(ports[i], pins[i], (instruction >> ((arr_size(pins) - 1) - i)) & 0x1);
	}
	//After RS and RW are set to desired values, tAS = 40 ns min needs to pass before enable pin is set HIGH.
	//Our resolution is in us, so wait 1 us.
	uint32_t tAS = 1;
	DWT_delay_us(tAS);

	//Toggle enable pin
	HAL_GPIO_WritePin(Pin_EN_GPIO_Port, Pin_EN_Pin, GPIO_PIN_SET);
	//After enable pin is set HIGH, tDSW = 80 ns min needs to pass.
	//Our resolution is in microseconds, so wait 1 us.
	uint32_t tDSW = 1;
	DWT_delay_us(tDSW);
	HAL_GPIO_WritePin(Pin_EN_GPIO_Port, Pin_EN_Pin, GPIO_PIN_RESET);
	//After enable pin is set LOW, both the address and the data lines are held by the chip
	//for tAH and tH respectively, both of them are 10 ns min. Waiting 1 microsecond is very wasteful, so
	//just wait for 100 clock cycles using NOP.
	for (int i = 0; i < 100; i++)
	{
		__NOP();
	}
}

void Init16x2LCD()
{
	/*
		HD44780U has an internal reset circuitry that automatically initializes the screen
		on power on. During this, the busy flag is kept at high. The datasheet specifies
		this initialization takes 10ms after Vcc rises to 4.5V. If the busy flag isn't
		turned off by then, the MCU needs to do the initialization manually. This can
		happen when power supply can't supply the expectations of the chip. For simplicity,
		this check will be omitted here. A complete implementation should implement this
		check.
	*/
	//Give the chip 10ms to complete internal initialization + 2ms headroom.
	//If this function isn't called when the system is starting, this delay won't be necessary. But
	//just to make sure this function works no matter where it is called from, introduce a delay anyways.
	HAL_Delay(12);

	//Enable this before sending any instructions because instruction sending
	//relies on microsecond delays, for which DWT needs to be enabled.
	DWT_Init();

	FunctionSet(1, 1, 0);
	DisplayAndCursorControl(1, 0, 0);
	EntryModeSet(1, 0);
	ClearScreen();
}

void ClearScreen()
{
	SendInstruction(0b0000000001);
}

void ReturnHome()
{
	SendInstruction(0b0000000010);
}

void EntryModeSet(uint8_t increment, uint8_t shiftDisplay)
{
	uint16_t instruction = 0b0000000100;
	if (increment)
	{
		instruction |= (1 << 1);
	}
	if (shiftDisplay)
	{
		instruction |= (1 << 0);
	}
	SendInstruction(instruction);
}

void DisplayAndCursorControl(uint8_t display, uint8_t cursor, uint8_t blink)
{
	uint16_t instruction = 0b0000001000;
	if (display)
	{
		instruction |= (1 << 2);
	}
	if (cursor)
	{
		instruction |= (1 << 1);
	}
	if (blink)
	{
		instruction |= (1 << 0);
	}
	SendInstruction(instruction);
}

void ShiftCursor(uint8_t shiftRight)
{
	uint16_t instruction = 0b0000010000;
	if (shiftRight)
	{
		instruction |= (1 << 2);
	}
	SendInstruction(instruction);
}

void MoveCursor(uint8_t line, uint8_t position)
{
	static const uint8_t arr[2] = { FIRST_LINE_START_ADDRESS_IN_DDRAM, SECOND_LINE_START_ADDRESS_IN_DDRAM };

	if (line < 1)
	{
		line = 1;
	}
	else if (line > 2)
	{
		line = 2;
	}

	if (position < 1)
	{
		position = 1;
	}
	else if (position > 40)
	{
		position = 40;
	}

	SetDDRAMAddress(arr[line - 1] + position - 1); //Subtract 1 because the addresses start from 0 and the screen lines and rows start from 1.
}

uint8_t GetCurrentLine()
{
	uint8_t ac = ReadAddressCounter();
	if (ac >= FIRST_LINE_START_ADDRESS_IN_DDRAM && ac <= FIRST_LINE_END_ADDRESS_IN_DDRAM)
	{
		return 1;
	}
	else if (ac >= SECOND_LINE_START_ADDRESS_IN_DDRAM && ac <= SECOND_LINE_END_ADDRESS_IN_DDRAM)
	{
		return 2;
	}
	return 255;
}

void ShiftDisplay(uint8_t shiftRight)
{
	uint16_t instruction = 0b0000011000;
	if (shiftRight)
	{
		instruction |= (1 << 2);
	}
	SendInstruction(instruction);
}

void ShiftDisplayRight(size_t n)
{
	for (size_t i = 0; i < n; i++)
	{
		ShiftDisplay(1);
	}
}

void ShiftDisplayLeft(size_t n)
{
	for (size_t i = 0; i < n; i++)
	{
		ShiftDisplay(0);
	}
}

void FunctionSet(uint8_t using8Bits, uint8_t using2Lines, uint8_t using5x10Font)
{
	uint16_t instruction = 0b0000100000;
	if (using8Bits)
	{
		instruction |= (1 << 4);
	}
	if (using2Lines)
	{
		instruction |= (1 << 3);
	}
	if (using5x10Font)
	{
		instruction |= (1 << 2);
	}
	SendInstruction(instruction);
}

void SendByte(uint8_t byte)
{
	uint16_t instruction = 0b1000000000;
	instruction |= byte;
	SendInstruction(instruction);

	/*
	  This function is writing data to CGRAM or DDRAM. This internally updates the RAM address counter.
	  The update happens tADD us after the busy flag turns off. Wait for the busy flag to turn off and
	  wait for tADD us so that address counter becomes valid for future instructions.
	*/
	while (IsBusy()) {}
	uint32_t tADD = 4;
	DWT_delay_us(tADD);
}

void WriteCharacter(uint8_t character)
{
	SendByte(character);
}

void WriteString(const char* text)
{
	size_t len = strlen(text);
	for (size_t i = 0; i < len; i++)
	{
		WriteCharacter(text[i]);
	}
}

void SetCGRAMAddress(uint8_t address)
{
	uint16_t instruction = 0b0001000000;
	address &= 0x3F; //Zero out the highest two bits
	instruction |= address;
	SendInstruction(instruction);
}

void SetDDRAMAddress(uint8_t address)
{
	uint16_t instruction = 0b0010000000;
	address &= 0x7F; //Zero out the highest bit
	instruction |= address;
	SendInstruction(instruction);
}

uint8_t IsBusy()
{
	//Notify the chip we want to read the busy flag
	HAL_GPIO_WritePin(Pin_RS_GPIO_Port, Pin_RS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(Pin_RW_GPIO_Port, Pin_RW_Pin, GPIO_PIN_SET);
	uint8_t data = ReadLCDMemory_Internal();
	return (data >> 7); //highest bit is the busy flag
}

uint8_t ReadAddressCounter()
{
	//Notify the chip we want to read the address counter
	HAL_GPIO_WritePin(Pin_RS_GPIO_Port, Pin_RS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(Pin_RW_GPIO_Port, Pin_RW_Pin, GPIO_PIN_SET);

	//Wait until the busy flag turns off
	while (IsBusy()) {}
	uint32_t tADD = 4; //Address counter becomes valid tADD us after the busy flag turns off
	DWT_delay_us(tADD);

	uint8_t data = ReadLCDMemory_Internal();
	return data & 0x7F; //All bits except the highest one make up the address
}

uint8_t ReadByte()
{
	//Notify the chip we want to read the RAM
	HAL_GPIO_WritePin(Pin_RS_GPIO_Port, Pin_RS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Pin_RW_GPIO_Port, Pin_RW_Pin, GPIO_PIN_SET);

	//Wait until the busy flag turns off
	while (IsBusy()) {}
	uint32_t tADD = 4; //Address counter becomes valid tADD us after the busy flag turns off
	DWT_delay_us(tADD);

	return ReadLCDMemory_Internal();
}
