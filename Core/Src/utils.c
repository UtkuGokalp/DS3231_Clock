/*
 * utils.c
 *
 *  Created on: Jan 28, 2026
 *      Author: ugklp
 */

#include "utils.h"

uint8_t BinaryToBCD(uint8_t binary)
{
	if (binary > 99)
	{
		binary = 99;
	}
	return ((binary / 10) << 4) | (binary % 10);
}

uint8_t BCDToBinary(uint8_t bcd)
{
    uint8_t mostSigDig = (bcd & 0xF0) >> 4;
    uint8_t leastSigDig = bcd & 0x0F;
	if (mostSigDig > 9)
	{
	    mostSigDig = 9;
	}
	if (leastSigDig > 9)
	{
	    leastSigDig = 9;
	}
	return mostSigDig * 10 + leastSigDig;
}
