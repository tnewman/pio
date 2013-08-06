/*
 * File:        register.h
 * Description: Definition of register addresses and macros to perform simple register calculations.
 * Programmer:  Aug 1, 2013
 * Date:        tnewman
 */

#ifndef REGISTER_H
#define REGISTER_H

// Size of each Register
#define REGISTER_SIZE 32

// Number of Pins
#define GPIO_PIN_COUNT

// GPIO Memory Region
#define GPIO_MEMORY_START 0x20200000
#define GPIO_MEMORY_END 0x202000B0
#define GPIO_MEMORY_SIZE (GPIO_MEMORY_END - GPIO_MEMORY_START)

// GPIO Function Select Registers
#define GPFSEL0 0x20200000
#define GPFSEL1 0x20200004
#define GPFSEL2 0x20200008
#define GPFSEL3 0x2020000C
#define GPFSEL4 0x20200010
#define GPFSEL5 0x20200014
#define GPFSEL_BITS_PER_PIN 3

// GPIO Functions
#define GPIO_INPUT 0x00
#define GPIO_OUTPUT 0x01
#define GPIO_FUNCTION_0 0x04
#define GPIO_FUNCTION_1 0x05
#define GPIO_FUNCTION_2 0x06
#define GPIO_FUNCTION_3 0x07
#define GPIO_FUNCTION_4 0x03
#define GPIO_FUNCTION_5 0x02

// GPIO Set Registers
#define GPSET0 0x2020001C
#define GPSET1 0x20200020
#define GPSET_BITS_PER_PIN 1
#define GPSET_BITS 0x01

// GPIO Clear Registers
#define GPCLR0 0x20200028
#define GPCLR1 0x2020002C
#define GPCLR_BITS_PER_PIN 1
#define GPCLR_BITS 0x01

// GPIO Level Registers
#define GPLEV0 0x20200034
#define GPLEV1 0x20200038
#define GPLEV_BITS_PER_PIN 1

// Calculate Offset
#define CALCULATE_OFFSET(register_address) (((register_address) - GPIO_MEMORY_START) / 4)
	// Note: Since memory is integer aligned with byte-level addressing, it is necessary to divide by 4 to get
	//		 the actual offset.

#endif /* REGISTER_H_ */
