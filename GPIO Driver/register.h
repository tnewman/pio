/*
 * File:        register.h
 * Description: Definition of register addresses and macros to perform simple register calculations.
 * Programmer:  Aug 1, 2013
 * Date:        tnewman
 */

#ifndef REGISTER_H_
#define REGISTER_H_

// Number of Pins
#define GPIO_PIN_COUNT

// GPIO Memory Region
#define GPIO_MEMORY_START
#define GPIO_MEMORY_END

// GPIO Function Select Registers
#define GPFSEL0 0x7E200000
#define GPFSEL1 0x7E200004
#define GPFSEL2 0x7E200008
#define GPFSEL3 0x7E20000C
#define GPFSEL4 0x7E200010
#define GPFSEL5 0x7E200014

// GPIO Set Registers
#define GPSET0 0x7E20001C
#define GPSET1 0x7E200020

// GPIO Clear Registers
#define GPCLR0 0x7E200028
#define GPCLR1 0x7E20002C

// Calculate Offset
#define CALCULATE_OFFSET(register_address) (((register_address) - GPIO_MEMORY_START) / 4)
	// Note: Since memory is integer aligned with byte-level addressing, it is necessary to divide by 4 to get
	//		 the actual offset.

#endif /* REGISTER_H_ */
