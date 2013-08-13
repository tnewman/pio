/*
 * File:        gpio.h
 * Description: Definition of the GPIO API functions, data types, and constants.
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

#ifndef GPIO_H
#define GPIO_H

#define MEMORY_FILE "/dev/mem"
#define CPU_INFO_PATH "/proc/cpuinfo"
#define MAX_LINE_LENGTH 100
#define CHIPSET_HEADER "Hardware\t: "
#define CHIPSET "BCM2708"
#define REVISION_HEADER "Revision\t: "
#define REVISION_LENGTH 0x10000
#define REVISION_1_START 0x02
#define REVISION_2_START 0x04

typedef enum {
	BROADCOM, // The pin numbers in the Broadcom manual referenced by the Raspberry Pi's schematic.
	P1CONNECTOR // The physical pin numbers on the Raspberry Pi's P1 connector.
} PinType;

typedef enum {
	SUCCESS, // The function returned successfully.
	NOT_ROOT, // The executable is not seteuid root, and the executable was not run as root (either one will work).
	INVALID_CHIPSET, // The hardware is unreadable or not recognized by the program.
	CANNOT_MAP_MEMORY, // The GPIO memory cannot be mapped for an unknown reason (sorry, can't do any better).
	NO_INIT, // The init function has not been completed successfully.
	INVALID_PIN, // The pin number is not a pin number supported by the current pin type.
	REGISTER_FAILURE, // There was an internal problem setting a register
} StatusCode;

typedef struct {
    int physical_pin_number;
    int broadcom_pin_number;
} PhysicalPin;

/*
 * API Functions
 */
StatusCode initialize_gpio();
StatusCode finalize_gpio();
StatusCode set_gpio_pin(int pin_number, PinType pin_type);
StatusCode clear_gpio_pin(int pin_number, PinType pin_type);
StatusCode get_gpio_pin(int pin_number, PinType pin_type, int* pin_value);

#endif /* GPIO_H_ */
