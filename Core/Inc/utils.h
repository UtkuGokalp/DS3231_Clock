/*
 * utils.h
 *
 *  Created on: Jan 28, 2026
 *      Author: ugklp
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include <stdint.h>

/*
  Converts a binary value into BCD. If the value is larger than 99, 99 is returned in BCD.
  This is because 99 is the largest value representable in BCD using a single byte.
*/
uint8_t BinaryToBCD(uint8_t binary);

/*
  Converts a BCD value into binary. If one of digits in BCD is greater than 9, it will be
  calculated as 9. This is because there are no digits larger than 9.
*/
uint8_t BCDToBinary(uint8_t bcd);

#endif /* INC_UTILS_H_ */
