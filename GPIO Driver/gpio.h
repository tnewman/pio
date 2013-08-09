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
	NO_CPU_INFO, // The CPU information file cannot be opened
	INVALID_CHIPSET, // The hardware is not a Raspberry Pi.
	INVALID_REVISION, // The hardware revision is not recognized by the program.
	CANNOT_MAP_MEMORY, // The GPIO memory cannot be mapped for an unknown reason (sorry, can't do any better).
	NO_INIT, // The init function has not been completed successfully.
	INVALID_TYPE, // The pin type is not any of the allowed pin types.
	INVALID_PIN_NUMBER, // The pin number is not a pin number supported by the current pin type.
	INVALID_FUNCTION_CODE, // The supplied function code exceeds the allowable number of bits
} StatusCode;

StatusCode initialize_gpio();
StatusCode finalize_gpio();
StatusCode set_gpio_pin(int pin_number, PinType pin_type);
StatusCode clear_gpio_pin(int pin_number, PinType pin_type);
StatusCode get_gpio_pin(int pin_number, PinType pin_type, int* pin_value);

#endif /* GPIO_H_ */
