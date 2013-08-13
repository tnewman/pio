pio
===

PIo is short for Raspberry Pi GPIO. This C library allows the Raspberry Pi's GPIO pins to be accessed through a 
C library.

## Features

* Simple API (5 public functions).
* Fast (writes bits directly to the GPIO registers).
* No external dependencies outside of the Linux C libraries.
* Set, clear, and read status from any valid GPIO pin.
* Allows GPIO pins on the GPIO connector to be referenced by position on the connector.
* Changes the mapping of the GPIO pins on the P1 connector to the Broadcom pins based on hardware revision.

## Usage

* StatusCode initialize_gpio(); - Maps the GPIO memory and verifies that a Raspberry Pi with a known revision is 
                                  running the library (run first).
* StatusCode set_gpio_pin(int pin_number, PinType pin_type); - Sets a given pin high.
* StatusCode clear_gpio_pin(int pin_number, PinType pin_type); - Clears a given pin.
* StatusCode get_gpio_pin(int pin_number, PinType pin_type, int* pin_value); - Gets the value of a given pin.
* StatusCode finalize_gpio(); - Unmaps the GPIO memory (always run once the library is no longer needed).

### Note:
* It is the responsibility of the user to make sure initialize_gpio() returns success before attempting 
to use set_gpio_pin, clear_gpio_pin, or get_gpio_pin, or these functions will return a failure status to 
avoid unexpected results.
* It is the responsibility of the user to make sure finalize_gpio() is called before the program exits. Failure 
* to do so will leave the GPIO registers mapped in memory after the program exits.

## Data Type Description

* PinType - Specifies the type of connector that the pin numbers are referencing.
 * BROADCOM - The pin numbers in the Broadcom manual referenced by the Raspberry Pi's schematic.
 * P1CONNECTOR - The physical pin numbers on the Raspberry Pi's P1 connector.
* StatusCode - Specifies the outcome of calling an API function.
 * SUCCESS - The function returned successfully.
 * NOT_ROOT - The executable is not seteuid root, and the executable was not run as root (either one will work).
 * INVALID_CHIPSET - The hardware is unreadable or not recognized by the program.
 * CANNOT_MAP_MEMORY - The GPIO memory cannot be mapped for an unknown reason (sorry, can't do any better).
 * NO_INIT - The init function has not been completed successfully.
 * INVALID_PIN - The pin number is not a pin number supported by the current pin type.
 * REGISTER_FAILURE - There was an internal problem setting a register

## Regarding Root Access

In order to map the register memory, the library requires root access. Simply running the entire program as root 
is a poor idea because it can introduce a severe security hole. Instead, the program should be owned by the root 
user with the seteuid permission bit set. This allows anyone who runs the program to have the permissions of the 
file's owner. The program will begin running as root. During this time, all of the program's tasks that require 
root privleges (initialize_gpio() in the case of this library) should be run. Once these tasks are complete, 
run the system call to change the effective user id to the actual user id (the user who ran the file). This allows 
the remainder of the program to run as the non-root user, restricting any risk of security vulnerabilities to the 
code that absolutely requires root. Although running as root avoids all of these steps, it is not recommended 
practice.
