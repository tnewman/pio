/*
 * File:        register.h
 * Description: Definition of register addresses and macros to perform simple register calculations.
 * Programmer:  Aug 1, 2013
 * Date:        tnewman
 */

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Thomas Newman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef REGISTER_H
#define REGISTER_H

// GPIO Register Type
typedef volatile unsigned int Register_Type;

// GPIO Pins
#define GPIO_PIN_COUNT 53

// Size of each Register
#define REGISTER_SIZE 32

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
#define CALCULATE_OFFSET(register_address) (((Register_Type) (register_address) - GPIO_MEMORY_START) / sizeof(Register_Type))

#endif /* REGISTER_H_ */
