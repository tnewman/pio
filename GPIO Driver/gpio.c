/*
 * File:        gpio.c
 * Description: Implementation of GPIO functions.
 * Programmer:  Aug 1, 2013
 * Date:        tnewman
 */

#include "gpio.h"
#include "register.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/mman.h>

// Each pin in the table corresponds to the pin in the same index of the other tables.
// Unfortunately, binary searches cannot be recommended because the tables always have
// a chance of going out of numerical order with a future hardware revision.

// Physical Pin Numbers - This table MUST be left in order for the binary search to work
const int PHYSICAL_PINS[] = { 3,  5,  7,  8, 10, 11, 12, 13, 15, 16, 18, 19, 21, 22, 23, 24, 26};

// Pin numbers for board revision 1 -
const int REV_1_PINS[] =    { 0,  1,  4, 14, 15, 17, 18, 21, 22, 23, 24, 10,  9, 25, 11,  8,  7};

// Pin numbers for board revision 2
const int REV_2_PINS[] =    { 2,  3,  4, 14, 15, 17, 18, 27, 22, 23, 24, 10,  9, 25, 11,  8,  7};

/*
 * Declaration Section
 */

static volatile void* gpio_memory = 0x00;
static unsigned int board_revision = 0x00;

/*
 * Name:        check_root
 * Description: Verifies that the executable is running as root or is seteuid root.
 * Inputs:      None
 * Returns:     SUCCESS or NOT_ROOT.
 */
static StatusCode check_root();

/*
 * Name:        check_system
 * Description: Verifies that the executable is running on a Raspberry Pi with a known board revision.
 * Inputs:      None
 * Returns:     SUCCESS, INVALID_CHIPSET, or INVALID_REVISION
 */
static StatusCode check_system();

/*
 * Name:        set_function_register
 * Description: Sets the function register for a given pin to the desired value.
 * Inputs:      broadcom pin number to set, function code
 * Returns:     SUCCESS, INVALID_PIN
 */
static StatusCode set_function_register(int broadcom_pin, int function_code);

/*
 * Name:        get_broadcom_pin
 * Description: Returns the broadcom pin number if the revision and pin are correct. Converts a
 *              physical pin number to the Broadcom pin number for the given Raspberry Pi
 *              revision. This function depends on the board revision global being populated.
 * Inputs:      pin number to convert, pin type, and broadcom pin (returned by reference)
 * Returns:     INVALID_TYPE, INVALID_REVISION, INVALID_PIN_TYPE, INVALID_PIN_NUMBER
 */
static StatusCode get_broadcom_pin(int pin_number, PinNumType pin_type, int* broadcom_pin);

/*
 * Implementation Section
 */

StatusCode map_gpio_memory()
{
    int memory_file;

    // Make sure the user is root before continuing
    if(check_root() == NOT_ROOT)
    {
        return NOT_ROOT;
    }

    // Map GPIO memory
    memory_file = open(MEMORY_FILE, O_RDWR);

    // Make sure the file opened correctly
    if(memory_file < 0)
    {
        return CANNOT_MAP_MEMORY;
    }

    gpio_memory = mmap(0x00, GPIO_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memory_file, GPIO_MEMORY_START);

    close(memory_file);

    // Failure if the memory location is 0x00
    if(gpio_memory == MAP_FAILED)
    {
        return CANNOT_MAP_MEMORY;
    }

    // At this point, the memory was mapped successfully
    return SUCCESS;
}

StatusCode set_pin(int pin_number, PinNumType pin_type)
{
    StatusCode status;
    int broadcom_pin;
    int set_register;
    int bit_offset;

    // Verify that memory is mapped
    if(gpio_memory == 0x00)
    {
        return MEMORY_NOT_MAPPED;
    }

    // Make sure the hardware is a Raspberry Pi with a known hardware revision
    status = check_system();

    if(status != SUCCESS)
    {
        return status;
    }

    status = get_broadcom_pin(pin_number, pin_type, &broadcom_pin);

    if(status != SUCCESS)
    {
        return status;
    }

    status = set_function_register(broadcom_pin, GPIO_OUTPUT);

    if(status != SUCCESS)
    {
        return status;
    }

    // Set the given pin
    switch((int) broadcom_pin / (REGISTER_SIZE / GPSET_BITS_PER_PIN))
    {
    case 0:
        set_register = GPSET0;
        break;
    case 1:
        set_register = GPSET1;
        break;
    default:
        return INVALID_PIN_NUMBER;
        break;
    }

    bit_offset = (broadcom_pin % (REGISTER_SIZE / GPSET_BITS_PER_PIN)) * GPSET_BITS_PER_PIN;

    *((volatile int*) gpio_memory + CALCULATE_OFFSET(set_register)) = GPSET_BITS << bit_offset;

    return SUCCESS;
}

StatusCode clear_pin(int pin_number, PinNumType pin_type)
{
    StatusCode status;
    int broadcom_pin;
    int clear_register;
    int bit_offset;

    // Verify that memory is mapped
    if(gpio_memory ==0x00)
    {
        return MEMORY_NOT_MAPPED;
    }

    // Make sure the hardware is a Raspberry Pi with a known hardware revision
    status = check_system();

    if(status != SUCCESS)
    {
        return status;
    }

    status = get_broadcom_pin(pin_number, pin_type, &broadcom_pin);

    if(status != SUCCESS)
    {
        return status;
    }

    status = set_function_register(broadcom_pin, GPIO_OUTPUT);

    if(status != SUCCESS)
    {
        return status;
    }

    // Clear the given pin
    switch((int) broadcom_pin / (REGISTER_SIZE / GPCLR_BITS_PER_PIN))
    {
    case 0:
        clear_register = GPCLR0;
        break;
    case 1:
        clear_register = GPCLR1;
        break;
    default:
        return INVALID_PIN_NUMBER;
        break;
    }

    bit_offset = (broadcom_pin % (REGISTER_SIZE / GPCLR_BITS_PER_PIN)) * GPCLR_BITS_PER_PIN;

    *((volatile int*) gpio_memory + CALCULATE_OFFSET(clear_register)) = GPCLR_BITS << bit_offset;

    return SUCCESS;
}

StatusCode get_pin(int pin_number, PinNumType pin_type, int* pin_status)
{
    StatusCode status;
    int broadcom_pin;
    int status_register;
    int bit_offset;

    // Verify that memory is mapped
    if(gpio_memory ==0x00)
    {
        return MEMORY_NOT_MAPPED;
    }

    // Make sure the hardware is a Raspberry Pi with a known hardware revision
    status = check_system();

    if(status != SUCCESS)
    {
        return status;
    }

    status = get_broadcom_pin(pin_number, pin_type, &broadcom_pin);

    if(status != SUCCESS)
    {
        return status;
    }

    status = set_function_register(broadcom_pin, GPIO_OUTPUT);

    if(status != SUCCESS)
    {
        return status;
    }

    // Get the given pin
    switch((int) broadcom_pin / (REGISTER_SIZE / GPCLR_BITS_PER_PIN))
    {
    case 0:
        status_register = GPLEV0;
        break;
    case 1:
        status_register = GPLEV1;
        break;
    default:
        return INVALID_PIN_NUMBER;
        break;
    }

    bit_offset = ((int) broadcom_pin % (REGISTER_SIZE / GPLEV_BITS_PER_PIN)) * GPLEV_BITS_PER_PIN;

    *pin_status = *((volatile int*) gpio_memory + CALCULATE_OFFSET(status_register)) >> bit_offset;

    return SUCCESS;
}

static StatusCode set_function_register(int broadcom_pin, int function_code)
{
    int function_register;
    int bit_offset;

    if(function_code / 8)
    {
        return INVALID_FUNCTION_CODE;
    }

    // Set the correct function select register
    switch((int) broadcom_pin / (REGISTER_SIZE / GPFSEL_BITS_PER_PIN))
    {
    case 0:
        function_register = GPFSEL0;
        break;
    case 1:
        function_register = GPFSEL1;
        break;
    case 2:
        function_register = GPFSEL2;
        break;
    case 3:
        function_register = GPFSEL3;
        break;
    case 4:
        function_register = GPFSEL4;
        break;
    case 5:
        function_register = GPFSEL5;
        break;
    default:
        return INVALID_PIN_NUMBER;
    }

    bit_offset = ((int) broadcom_pin % (REGISTER_SIZE / GPFSEL_BITS_PER_PIN)) * GPFSEL_BITS_PER_PIN;

    // The bits need to be cleared before they can be set again
    *((volatile int*) gpio_memory + CALCULATE_OFFSET(function_register)) &= ~(0x07 << bit_offset);

    // Now set the bits
    *((volatile int*) gpio_memory + CALCULATE_OFFSET(function_register)) |= function_code << bit_offset;

    return SUCCESS;
}

static StatusCode get_broadcom_pin(int pin_number, PinNumType pin_type, int* broadcom_pin)
{
    const int* broadcom_pins;
    int i;

    if(pin_type != BROADCOM && pin_type != P1CONNECTOR)
    {
        return INVALID_TYPE;
    }

    if(board_revision == 1)
    {
        broadcom_pins = REV_1_PINS;
    }
    else if(board_revision == 2)
    {
        broadcom_pins = REV_2_PINS;
    }
    else
    {
        return INVALID_REVISION;
    }

    // Get the Broadcom pin for the P1Connector
    if(pin_type == P1CONNECTOR)
    {
        for(i = 0; i < sizeof(PHYSICAL_PINS) / sizeof(int); i++)
        {
            /*
             * A match has been found, so return the corresponding Broadcom Pin that is stored
             * at the same index
             */
            if(pin_number == PHYSICAL_PINS[i])
            {
                *broadcom_pin = broadcom_pins[i];
                return SUCCESS;
            }
        }
    }
    // Verify that the Broadcom pin exists
    else if(pin_type == BROADCOM)
    {
        for(i = 0; i < sizeof(PHYSICAL_PINS) / sizeof(int); i++)
        {
            if(pin_number == broadcom_pins[i])
            {
                *broadcom_pin = broadcom_pins[i];
                return SUCCESS;
            }
        }
    }

    return INVALID_PIN_NUMBER;
}

StatusCode unmap_gpio_memory()
{
    // Memory must be mapped to be unmapped
    if(gpio_memory != 0x00)
    {
        munmap((void*) gpio_memory, GPIO_MEMORY_SIZE);
        gpio_memory = 0x00;
    }

    return SUCCESS;
}

static StatusCode check_root()
{
    if(geteuid() == 0 || getuid() == 0)
    {
        return SUCCESS;
    }
    else
    {
        return NOT_ROOT;
    }
}

static StatusCode check_system()
{
    FILE* cpu_info_file;
    char line_stream[LINE_MAX + 1];
    char chipset[LINE_MAX + 1] = "";
    unsigned int revision = 0;

    cpu_info_file = fopen(CPU_INFO_FILE, "r");

    // Failure if the CPU information file cannot be opened
    if(cpu_info_file == NULL)
    {
        return NO_CPU_INFO;
    }

    // Iterate through each line of the file
    // Subtract 1 to fix \0
    while(fgets(line_stream, LINE_MAX, cpu_info_file))
    {
        // Capture the chipset if it is on this line
        sscanf(line_stream, "Hardware\t: %s", chipset);

        // Capture the hardware revision if it is on this line
        sscanf(line_stream, "CPU revision\t: %x", &revision);
    }

    fclose(cpu_info_file);

    // Check if the chipset is a Raspberry Pi Chipset
    if(strcmp(CPU_FAMILY, chipset) != 0)
    {
        return INVALID_CHIPSET;
    }

    // Check for the hardware revision
    if(revision >= REV_1_START && revision < REV_2_START)
    {
        board_revision = 1;
    }
    else if(revision >= REV_2_START)
    {
        board_revision = 2;
    }
    else
    {
        return INVALID_REVISION;
    }

    return SUCCESS;
}
