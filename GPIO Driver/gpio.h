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

#ifndef GPIO_H
#define GPIO_H

/*
 * Global Declarations (API) Section
 */

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
	NO_CPU_INFO, // The CPU information file cannot be opened
	INVALID_CHIPSET, // The hardware is not a Raspberry Pi.
	INVALID_REVISION, // The hardware revision is not recognized by the program.
	CANNOT_MAP_MEMORY, // The GPIO memory cannot be mapped for an unknown reason (sorry, can't do any better).
	MEMORY_NOT_MAPPED, // The GPIO memory region has not been mapped yet
	INVALID_TYPE, // The pin type is not any of the allowed pin types.
	INVALID_PIN_NUMBER, // The pin number is not a pin number supported by the current pin type.
	INVALID_FUNCTION_CODE, // The supplied function code exceeds the allowable number of bits
} StatusCode;

/*
 * Name:        map_gpio_memory
 * Description: Maps the GPIO memory region. The memory will not be mapped again if it has already been
 *              allocated.
 * Inputs:      None
 * Returns:     SUCCESS, NO_CPU_INFO, INVALID_CHIPSET, INVALID_REVISION, or CANNOT_MAP_MEMORY
 */
StatusCode map_gpio_memory();

/*
 * Name:        set_pin
 * Description: Sets the specified GPIO pin.
 * Inputs:      pin number and pin numbering type
 * Returns:     SUCCESS, MEMORY_NOT_MAPPED, INVALID_CHIPSET, INVALID_REVISION, INVALID_TYPE, or INVALID_PIN_NUMBER
 */
StatusCode set_pin(int pin_number, PinNumType pin_type);

/*
 * Name:        clear_pin
 * Description: Clears the specified GPIO pin.
 * Inputs:      pin number and pin numbering type
 * Returns:     SUCCESS, MEMORY_NOT_MAPPED, INVALID_CHIPSET, INVALID_REVISION, INVALID_TYPE, or INVALID_PIN_NUMBER
 */
StatusCode clear_pin(int pin_number, PinNumType pin_type);

/*
 * Name:        get_pin
 * Description: Gets the value of the specified GPIO pin.
 * Inputs:      pin number, pin numbering type, and pin_valid (by reference - only valid if SUCCESS returned)
 * Returns:     SUCCESS, MEMORY_NOT_MAPPED, INVALID_CHIPSET, INVALID_REVISION, INVALID_TYPE, or INVALID_PIN_NUMBER
 */
StatusCode get_pin(int pin_number, PinNumType pin_type, int* pin_status);

/*
 * Name:        unmap_gpio_memory
 * Description: Unmaps the GPIO memory region. Does nothing if no memory is mapped.
 * Inputs:      None
 * Returns:     SUCCESS
 */
StatusCode unmap_gpio_memory();

/*
 * Configuration Section
 *
 * This section does not need to be modified to use the API but can be used to adapt to changing
 * hardware revisions.
 */

// Location of the file storing physical memory
#define MEMORY_FILE "/dev/mem"

// Location of the file storing Linux CPU information
#define CPU_INFO_FILE "/proc/cpuinfo"

// System on a Chip CPU Family
#define CPU_FAMILY "BCM2708"

// For the revisions, an assumption was made that the revision 2 layout will last indefinitely. This
// was based on the fact that there has not been another change as of August 2013. By making this
// assumption, the library will not need to be changed if the hardware revision ID changes and the
// board revision does not.

// Revision ID Where Revision 1 Starts
#define REV_1_START 0x0002

// Revision ID Where Revision 2 Starts
#define REV_2_START 0x0004

#endif /* GPIO_H_ */
