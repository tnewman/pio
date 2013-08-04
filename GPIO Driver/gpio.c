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
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/mman.h>

/*
 * Declaration Section
 */

static volatile void* gpio_memory = 0x00;
static unsigned int board_revision;

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
 * Name:        check_pin
 * Description: Verifies that the pin number supplied is a valid pin number for the given pin type.
 * Inputs:      pin number to check, pin type to check against
 * Returns:     SUCCESS, INVALID_REVISION, INVALID_TYPE, or INVALID_PIN_NUMBER
 */
static StatusCode check_pin(int pin_number, PinNumType pin_type);

/*
 * Implementation Section
 */

/*
 * Name:        map_gpio_memory
 * Description: Maps the GPIO memory region. The memory will not be mapped again if it has already been
 *              allocated.
 * Inputs:      None
 * Returns:     SUCCESS or CANNOT_MAP_MEMORY
 */
StatusCode map_gpio_memory()
{
    StatusCode system_result;
    int memory_file;

    // Make sure the user is root before continuing
    if(check_root() == NOT_ROOT)
    {
        return NOT_ROOT;
    }

    // Make sure the hardware is a Raspberry Pi with a known hardware revision
    system_result = check_system();

    if(system_result != SUCCESS)
    {
        return system_result;
    }

    // Map GPIO memory
    memory_file = open(MEMORY_FILE, O_RDONLY);

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

/*
 * Name:        set_pin
 * Description: Sets the specified GPIO pin.
 * Inputs:      pin number and pin numbering type
 * Returns:     SUCCESS, MEMORY_NOT_MAPPED, INVALID_TYPE, or INVALID_PIN_NUMBER
 */
StatusCode set_pin(int pin_number, PinNumType pin_type)
{
    // TODO: Combine functionality
}

/*
 * Name:        clear_pin
 * Description: Clears the specified GPIO pin.
 * Inputs:      pin number and pin numbering type
 * Returns:     SUCCESS, MEMORY_NOT_MAPPED, INVALID_TYPE, or INVALID_PIN_NUMBER
 */
StatusCode clear_pin(int pin_number, PinNumType pin_type)
{
}

/*
 * Name:        get_pin
 * Description: Gets the value of the specified GPIO pin.
 * Inputs:      pin number, pin numbering type, and pin_valid (by reference - only valid if SUCCESS returned)
 * Returns:     SUCCESS, MEMORY_NOT_MAPPED, INVALID_TYPE, or INVALID_PIN_NUMBER
 */
StatusCode get_pin(int pin_number, PinNumType pin_type)
{
}

/*
 * Name:        unmap_gpio_memory
 * Description: Unmaps the GPIO memory region. Does nothing if no memory is mapped.
 * Inputs:      None
 * Returns:     SUCCESS
 */
StatusCode unmap_gpio_memory()
{
    // Memory must be mapped to be unmapped
    if(gpio_memory != 0x00)
    {
        munmap(gpio_memory, GPIO_MEMORY_SIZE);
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
    void* line_stream = malloc(LINE_MAX);
    char chipset[LINE_MAX];
    unsigned int revision = 0;

    cpu_info_file = fopen(CPU_INFO_FILE, "r");

    // Failure if the CPU information file cannot be opened
    if(cpu_info_file == NULL)
    {
        return NO_CPU_INFO;
    }

    // Iterate through each line of the file
    while(fread(line_stream, LINE_MAX, 1, cpu_info_file) != -1)
    {
        // Capture the chipset if it is on this line
        fscanf(line_stream, "Hardware\t: %s", chipset);

        // Capture the hardware revision if it is on this line
        fscanf(line_stream, "Revision\t: %x", &revision);
    }

    fclose(cpu_info_file);
    free(line_stream);

    // Check if the chipset is a Raspberry Pi Chipset
    if(strcmp(CPU_FAMILY, chipset) != 0)
    {
        return INVALID_CHIPSET;
    }

    // Check for the hardware revision
    if(revision >= REV_1_START && revision < REV_2_START)
    {
        board_revision = revision;
    }
    else if(revision >= REV_2_START)
    {
        board_revision = revision;
    }
    else
    {
        return INVALID_REVISION;
    }

    return SUCCESS;
}

static StatusCode check_pin(int pin_number, PinNumType pin_type)
{
    int revision_id;
    int i;

    // Check for the hardware revision
    if(board_revision >= REV_1_START && board_revision < REV_2_START)
    {
        revision_id = 1;
    }
    else if(board_revision >= REV_2_START)
    {
        revision_id = 2;
    }
    else
    {
        return INVALID_REVISION;
    }

    // Complexity Note: Yes, this uses O(N) complexity; however doing so avoided
    // The need to use more complex data structures for pins (since the pins are
    // not necessarily in numerical order for Broadcom). The size of the data
    // structure will also be limited by the size of the connector on the board,
    // so the O(N) complexity is acceptable, given the small size of N that will
    // not increase by very much.

    // Broadcom Pin Numbering
    if(pin_type == BROADCOM)
    {
        if(revision_id == 1)
        {
            // Search Revision 1 Broadcom
            for(i = 0; i < sizeof(REV_1_PINS) / sizeof(int); i++)
            {
                if(REV_1_PINS[i] == pin_number)
                {
                    return SUCCESS;
                }
            }
        }
        // Search Revision 2 Broadcom
        else if(revision_id == 2)
        {
            for(i = 0; i < sizeof(REV_2_PINS) / sizeof(int); i++)
            {
                if(REV_2_PINS[i] == pin_number)
                {
                    return SUCCESS;
                }
            }
        }

    }
    // Physical Pin Numbering
    else if(pin_type == P1CONNECTOR)
    {
        // Search Physical
        for(i = 0; i < sizeof(PHYSICAL_PINS) / sizeof(int); i++)
        {
            if(PHYSICAL_PINS[i] == pin_number)
            {
                return SUCCESS;
            }
        }

    }

    return INVALID_PIN_NUMBER;
}

int main()
{
    return 0;
}
