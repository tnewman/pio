/*
 * File:        gpio.c
 * Description: Implementation of GPIO functions.
 * Programmer:  Aug 1, 2013
 * Date:        tnewman
 */

#include "gpio.h"
#include "register.h"

static volatile void* gpio_memory = 0x00;

/*
 * Name:        check_root
 * Description: Verifies that the executable is running as root or is seteuid root.
 * Inputs:      None
 * Returns:     SUCCESS or NOT_ROOT.
 */
StatusCode check_root();

/*
 * Name:        check_system
 * Description: Verifies that the executable is running on a Raspberry Pi with a known board revision.
 * Inputs:      None
 * Returns:     SUCCESS, INVALID_CHIPSET, or INVALID_REVISION
 */
StatusCode check_system();

/*
 * Name:        check_pin
 * Description: Verifies that the pin number supplied is a valid pin number for the given pin type.
 * Inputs:      pin number to check, pin type to check against
 * Returns:     SUCCESS, INVALID_TYPE, or INVALID_PIN_NUMBER
 */
StatusCode check_pin(int pin_number, PinNumType pin_type);
