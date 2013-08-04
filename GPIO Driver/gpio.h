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
 * Returns:     SUCCESS, MEMORY_NOT_MAPPED, INVALID_TYPE, or INVALID_PIN_NUMBER
 */
StatusCode set_pin(int pin_number, PinNumType pin_type);

/*
 * Name:        clear_pin
 * Description: Clears the specified GPIO pin.
 * Inputs:      pin number and pin numbering type
 * Returns:     SUCCESS, MEMORY_NOT_MAPPED, INVALID_TYPE, or INVALID_PIN_NUMBER
 */
StatusCode clear_pin(int pin_number, PinNumType pin_type);

/*
 * Name:        get_pin
 * Description: Gets the value of the specified GPIO pin.
 * Inputs:      pin number, pin numbering type, and pin_valid (by reference - only valid if SUCCESS returned)
 * Returns:     SUCCESS, MEMORY_NOT_MAPPED, INVALID_TYPE, or INVALID_PIN_NUMBER
 */
StatusCode get_pin(int pin_number, PinNumType pin_type);

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
#define CPU_FAMILY "BCM2835"

// For the revisions, an assumption was made that the revision 2 layout will last indefinitely. This
// was based on the fact that there has not been another change as of August 2013. By making this
// assumption, the library will not need to be changed if the hardware revision ID changes and the
// board revision does not.

// Revision ID Where Revision 1 Starts
#define REV_1_START 0x0002

// Revision ID Where Revision 2 Starts
#define REV_2_START 0x0004

// Each pin in the table corresponds to the pin in the same index of the other tables.
// Unfortunately, binary searches cannot be recommended because the tables always have
// a chance of going out of numerical order with a future hardware revision.

// Physical Pin Numbers - This table MUST be left in order for the binary search to work
const int PHYSICAL_PINS[] = { 3,  5,  7,  8, 10, 11, 12, 13, 15, 16, 18, 19, 21, 22, 23, 24, 26};

// Pin numbers for board revision 1 -
const int REV_1_PINS[] =    { 0,  1,  4, 14, 15, 17, 18, 21, 22, 23, 24, 10,  9, 25, 11,  8,  7};

// Pin numbers for board revision 2
const int REV_2_PINS[] =    { 2,  3,  4, 14, 15, 17, 18, 27, 22, 23, 24, 10,  9, 25, 11,  8,  7};

#endif /* GPIO_H_ */
