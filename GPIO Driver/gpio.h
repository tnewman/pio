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

/*
 * Configuration
 */
#define MEMORY_FILE "/dev/mem"
#define CPU_INFO_PATH "/proc/cpuinfo"
#define MAX_LINE_LENGTH 100
#define CHIPSET_HEADER "Hardware\t: "
#define CHIPSET "BCM2708"
#define REVISION_HEADER "Revision\t: "
#define REVISION_LENGTH 0x10000
#define REVISION_1_START 0x02
#define REVISION_2_START 0x04

/*
 * Name: PinType
 * Description: PinType specifies whether the pin count is based on Broadcom's GPIO numbering or a physical connector 
 *              on the Raspberry Pi.
 */
typedef enum {
	BROADCOM, // The pin numbers in the Broadcom manual referenced by the Raspberry Pi's schematic.
	P1CONNECTOR // The physical pin numbers on the Raspberry Pi's P1 connector.
} PinType;


/*
 * Name: StatusCode
 * Description: StatusCode specifies the result of the operation.
 */
typedef enum {
	SUCCESS, // The function returned successfully.
	NOT_ROOT, // The executable is not seteuid root, and the executable was not run as root (either one will work).
	INVALID_CHIPSET, // The hardware is unreadable or not recognized by the program.
	CANNOT_MAP_MEMORY, // The GPIO memory cannot be mapped for an unknown reason (sorry, can't do any better).
	NO_INIT, // The init function has not been completed successfully.
	INVALID_PIN, // The pin number is not a pin number supported by the current pin type.
	REGISTER_FAILURE, // There was an internal problem setting a register
} StatusCode;

/*
 * Name: PhysicalPin
 * Description: Associates a physical pin number with the internal number used by the Broadcom CPU.
 */
typedef struct {
    int physical_pin_number;
    int broadcom_pin_number;
} PhysicalPin;

/*
 * API Functions
 */
 
/*
 * Name: initialize_gpio
 * Description: initialize_gpio checks to make sure that the effective user is root, makes sure the chipset is a 
 *              recognized Raspberry Pi, maps the memory of the Raspberry Pi's GPIO registers, and drops 
 *              privileges to the actual user ID.
 * Warning: The executable has to be owned by the root user with the seteuid bit set for memory to be mapped and 
 * privileges to be lowered once memory is mapped. The executable should be ran as an unprivileged user. 
 * Running as root is a security risk.
 * Parameters: None
 * Returns: Result of the operation.
 */
StatusCode initialize_gpio();

/*
 * Name: finalize_gpio
 * Description: finalize_gpio unmaps the memory location used by the Raspberry Pi's GPIO registers.
 * Note: finalize_gpio should be called once all GPIO access is done before the program terminates. If this function is 
 *       called, further gpio operations cannot take place because the root permissions needed to map the memory have 
 *       already been dropped.
 * Parameters: None
 * Returns: Result of the operation.
 */
StatusCode finalize_gpio();

/*
 * Name: set_gpio_pin
 * Description: Sets a GPIO.
 * Note: Must be called after initialize_gpio.
 * Parameters:
 *       pin_number[in] - The GPIO pin number to set.
 *       pin_type[in] - The numbering convention used to identify the GPIO pin.
 * Returns: Result of the operation.
 */
StatusCode set_gpio_pin(int pin_number, PinType pin_type);

/*
 * Name: clear_gpio_pin
 * Description: Clears a GPIO pin.
 * Note: Must be called after initialize_gpio.
 * Parameters:
 *       pin_number[in] - The GPIO pin number to clear.
 *       pin_type[in] - The numbering convention used to identify the GPIO pin.
 * Returns: Result of the operation.
 */
StatusCode clear_gpio_pin(int pin_number, PinType pin_type);

/*
 * Name: get_gpio_pin
 * Description: Gets the current value of a GPIO pin.
 * Note: Must be called after initialize_gpio.
 *       pin_number[in] - The GPIO pin number to retrieve a value from.
 *       pin_type[in] - The numbering convention used to identify the GPIO pin.
 *       pin_value[out] - The current value of the GPIO pin.
 * Returns: Result of the operation.
 */
StatusCode get_gpio_pin(int pin_number, PinType pin_type, int* pin_value);

#endif /* GPIO_H_ */
