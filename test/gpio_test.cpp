/*
 *	File:		gpio_test.cpp
 *	Description:	Tests GPIO memory access using Broadcom GPIO Pin 17 on revision 2
 *			of the Raspberry Pi
 *	Programmer:	Thomas Newman
 *	Date:		2013/07/30
 *
 *	Note:		This program must run as root to successfully map the file.
 *			This program is just to test the basic functionality. All it does
 *			is turn one LED on and then off again.
 *
 *	The MIT License (MIT)
 *
 *	Copyright (c) 2013 Thomas Newman
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy of
 *	this software and associated documentation files (the "Software"), to deal in
 *	the Software without restriction, including without limitation the rights to
 *	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 *	the Software, and to permit persons to whom the Software is furnished to do so,
 *	subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *	FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *	COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *	IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *	CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#define GPIO_START_ADDRESS 0x20200000
#define BLOCK_SIZE (4*1024)
// Offsets are required to convert byte level addressing to 4 byte addressing
#define GPIO_FUNC_1 0x04 / 4
#define GPIO_PIN_SET_1 0x1C / 4
#define GPIO_PIN_CLR_1 0x28 / 4

int main()
{
	int mapped_registers;
	volatile unsigned* register_address;

	// Map the GPIO memory
	mapped_registers = open("/dev/mem", O_RDWR | O_SYNC);
	register_address = (volatile unsigned*) mmap(NULL, BLOCK_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, mapped_registers, GPIO_START_ADDRESS);
	close(mapped_registers);

	// Clear the mode for the register
	*(register_address + GPIO_FUNC_1) &= 0xFF1FFFFF;

	// Set the register to output mode
	*(register_address + GPIO_FUNC_1) |= 0x200000;

	// Set the output
	*(register_address + GPIO_PIN_SET_1) = 0x20000;

	sleep(1);

	// Clear the output
	*(register_address + GPIO_PIN_CLR_1) = 0x20000;

	// Unmap the GPIO memory
	munmap((void*) register_address, BLOCK_SIZE);

	return 0;
}
