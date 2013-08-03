/*
 * File:        gpio.h
 * Description: Declaration of GPIO functions. This Header file serves as the API for the GPIO library.
 * Programmer:  Aug 1, 2013
 * Date:        tnewman
 */

/*
 * API Usage
 * map_gpio_memory to initialize (must return success to proceed)
 * set_pin to set a GPIO Pin
 * clear_pin to clear a GPIO Pin
 * unmap_gpio_memory to clean up (do not set or clear after calling this)
 */

#ifndef GPIO_H_
#define GPIO_H_

/*
 * PinNumType specifies the numbering convention of the GPIO pins.
 */
typedef enum {
	BROADCOM, // The pin numbers in the Broadcom manual referenced by the Raspberry Pi's schematic.
	P1CONNECTOR // The physical pin numbers on the Raspberry Pi's P1 connector.
} PinNumType;

/*
 * StatusCode specifies the codes that functions can return.
 */
typedef enum {
	SUCCESS, // The function returned successfully.
	NOT_ROOT, // The executable is not seteuid root, and the executable was not run as root (either one will work).
	INVALID_CHIPSET, // The hardware is not a Raspberry Pi.
	INVALID_REVISION, // The hardware revision is not recognized by the program.
	CANNOT_MAP_MEMORY, // The GPIO memory cannot be mapped for an unknown reason (sorry, can't do any better).
	MEMORY_NOT_MAPPED, // The GPIO memory region has not been mapped yet
	INVALID_TYPE, // The pin type is not any of the allowed pin types.
	INVALID_PIN_NUMBER, // The pin number is not a pin number supported by the current pin type.
} StatusCode;

/*
 * Name:        map_gpio_memory
 * Description: Maps the GPIO memory region. The memory will not be mapped again if it has already been
 *              allocated.
 * Inputs:      None
 * Returns:     SUCCESS or CANNOT_MAP_MEMORY
 */
StatusCode map_gpio_memory();

/*
 * Name:        set_pin
 * Description: Sets the specified GPIO pin.
 * Inputs:      pin number and pin numbering type
 * Returns:     SUCCESS, MEMORY_NOT_MAPPED, INVALID_TYPE, or INVALID_PIN_NUMBER
 */
StatusCode set_pin(int pin_number, PinNumType pin_type);

/*
 * Name:        clear_pin
 * Description: Clears the specified GPIO pin.
 * Inputs:      pin number and pin numbering type
 * Returns:     SUCCES, MEMORY_NOT_MAPPED, INVALID_TYPE, or INVALID_PIN_NUMBER
 */
StatusCode clear_pin(int pin_number, PinNumType pin_type);

/*
 * Name:        unmap_gpio_memory
 * Description: Unmaps the GPIO memory region. Does nothing if no memory is mapped.
 * Inputs:      None
 * Returns:     SUCCESS
 */
StatusCode unmap_gpio_memory();

#endif /* GPIO_H_ */
