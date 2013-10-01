/*
 * File:        gpio.c
 * Description: GPIO library implementation.
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

#include "gpio.h"
#include "register.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdbool.h>

/*
 * Global Variables
 */
volatile Register_Type* gpio_memory = 0x00; // Memory address of mapped GPIO memory
int revision = 0x00; // CPU Revision of the current Raspberry Pi

/*
 * Physical Pin Tables
 *
 * These tables map physical pin numbers for each supported revision to the actual pin
 * numbers used internally by Broadcom.
 */
const PhysicalPin REVISION_1_TABLE[] = {{3, 0}, {5, 1}, {7, 4}, {8, 14}, {10, 15}, {11, 17}, {12, 18},
        {13, 21}, {15, 22}, {16, 23}, {18, 24}, {19, 10}, {21, 9}, {22, 25}, {23, 11}, {24, 8}, {26, 7}};
const PhysicalPin REVISION_2_TABLE[] = {{3, 2}, {5, 3}, {7, 4}, {8, 14}, {10, 15}, {11, 17}, {12, 18},
        {13, 27}, {15, 22}, {16, 23}, {18, 24}, {19, 10}, {21, 9}, {22, 25}, {23, 11}, {24, 8}, {26, 7}};

/*
 * Implementation Functions
 *
 * These functions are for internal use only.
 */

/*
 * Name: map_memory
 * Description: Maps the GPIO register memory.
 * Parameters: None
 * Returns: true if memory is mapped successfully, otherwise false.
 */
static bool map_memory();

/*
 * Name: unmap_memory
 * Description: Unmaps the GPIO register memory.
 * Parameters: None
 * Returns: None
 */
static void unmap_memory();

/*
 * Name: check_root
 * Description: Checks if the effective user is root, so memory can be mapped.
 * Parameters: None
 * Returns: true if the effective user is root, otherwise false.
 */
static bool check_root();

/*
 * Name: set_cpu
 * Description: Sets the revision of the CPU used on this Raspberry Pi.
 * Parameters: None
 * Returns: true if the CPU revision is recognized and set, otherwise false.
 */
static bool set_cpu();

/*
 * Name: check_init
 * Description: Checks that the memory has been mapped and the CPU revision has 
 *              been set.
 * Parameters: None
 * Returns: true if memory is mapped and CPU revision is set, otherwise false.
 */
static bool check_init();

/*
 * Name: pin_to_broadcom
 * Description: Retrieves the Broadcom pin number based on the chosen numbering
 *              convention.
 * Parameters:
 *       pin_number[in] - The GPIO member to retrieve a value from.
 *       pin_type[in] - The numbering convention used to identify the GPIO pin.
 *       broadcom_number[out] - The Broadcom pin number of the supplied pin.
 * Returns: true if a match was found between the pin number and Broadcom number, 
 *          otherwise false.
 */
static bool pin_to_broadcom(int pin_number, PinType pin_type, int* broadcom_number);

/*
 * Name: pin_to_broadcom
 * Description: Verifies that a pin number is a valid Broadcom pin number.
 * Parameters:
 *       broadcom_number[in] - Broadcom pin number to check.
 * Returns: true if the pin is a Broadcom pin number, otherwise false.
 */
static bool check_broadcom_pin(int broadcom_number);

/*
 * Name: check_physical_pin
 * Description: Converts a physical pin number to a Broadcom pin number.
 * Parameters:
 *       physical_number[in] - Physical pin number to check.
 *       connector_type[in] - Numbering convention used for the supplied pin.
 *       broadcom_number[out] - Broadcom pin number associated with the physical pin.
 *     
 * Returns: true if the pin maps to a Broadcom pin number, otherwise false.
 */
static bool check_physical_pin(int physical_number, PinType connector_type, int* broadcom_number);

/*
 * Name: set_gpio_pin_function
 * Description: Converts a physical pin number to a Broadcom pin number.
 * Parameters:
 *       physical_number[in] - The Broadcom pin to set the register function of.
 *       function_code[in] - The register function number.
 * Returns: true if register function is changed successfully, otherwise false.
 */
static bool set_gpio_pin_function(int broadcom_number, int function_code);

StatusCode initialize_gpio()
{
    // Root permissions are necessary to map memory
    if(!check_root())
    {
        return NOT_ROOT;
    }

    // If the chipset is not recognized, the GPIO memory region cannot be mapped
    if(!set_cpu())
    {
        return INVALID_CHIPSET;
    }

    // GPIO is initialized once the memory is mapped successfully
    if(map_memory())
    {
        return SUCCESS;
    }
    else
    {
        return CANNOT_MAP_MEMORY;
    }
}

StatusCode finalize_gpio()
{
    unmap_memory();
    revision = 0x00;

    return SUCCESS;
}

StatusCode set_gpio_pin(int pin_number, PinType pin_type)
{
    bool status;
    int set_register;
    int bit_offset;
	int broadcom_number;

    // Check initialization
    if(!check_init())
    {
        return NO_INIT;
    }

    // Attempt to get the Broadcom pin number
    status = pin_to_broadcom(pin_number, pin_type, &broadcom_number);

    if(!status)
    {
        return INVALID_PIN;
    }

    // Set the pin function to output mode
    status = set_gpio_pin_function(broadcom_number, GPIO_OUTPUT);

    if(!status)
    {
        return REGISTER_FAILURE;
    }

    // Find the correct register
    switch((int) broadcom_number / (REGISTER_SIZE / GPSET_BITS_PER_PIN))
    {
    case 0:
        set_register = GPSET0;
        break;
    case 1:
        set_register = GPSET1;
        break;
    default:
        return REGISTER_FAILURE;
        break;
    }

    // Calculate the bit offset
    bit_offset = ((int) broadcom_number % (REGISTER_SIZE / GPSET_BITS_PER_PIN)) * GPSET_BITS_PER_PIN;
    // Set the pin
    *((Register_Type*) gpio_memory + CALCULATE_OFFSET(set_register)) = GPSET_BITS << bit_offset;

    return SUCCESS;
}

StatusCode clear_gpio_pin(int pin_number, PinType pin_type)
{
    bool status;
    int clear_register;
    int bit_offset;
	int broadcom_number;

    // Check initialization
    if(!check_init())
    {
        return NO_INIT;
    }

    // Attempt to get the Broadcom pin number
    status = pin_to_broadcom(pin_number, pin_type, &broadcom_number);

    if(!status)
    {
        return INVALID_PIN;
    }

    // Set the pin function to output mode
    status = set_gpio_pin_function(broadcom_number, GPIO_OUTPUT);

    if(!status)
    {
        return REGISTER_FAILURE;
    }

    // Find the correct register
    switch((int) broadcom_number / (REGISTER_SIZE / GPCLR_BITS_PER_PIN))
    {
    case 0:
        clear_register = GPCLR0;
        break;
    case 1:
        clear_register = GPCLR1;
        break;
    default:
        return REGISTER_FAILURE;
        break;
    }

    // Calculate the bit offset
    bit_offset = ((int) broadcom_number % (REGISTER_SIZE / GPCLR_BITS_PER_PIN)) * GPCLR_BITS_PER_PIN;

    // Clear the pin
    *((Register_Type*) gpio_memory + CALCULATE_OFFSET(clear_register)) = GPCLR_BITS << bit_offset;

    return SUCCESS;
}

StatusCode get_gpio_pin(int pin_number, PinType pin_type, int* pin_value)
{
    bool status;
    int status_register;
    int bit_offset;
	int broadcom_number;

    // Check initialization
    if(!check_init())
    {
        return NO_INIT;
    }

    // Attempt to get the Broadcom pin number
    status = pin_to_broadcom(pin_number, pin_type, &broadcom_number);

    if(!status)
    {
        return INVALID_PIN;
    }

    // Set the pin function to input mode
    status = set_gpio_pin_function(broadcom_number, GPIO_INPUT);

    if(!status)
    {
        return REGISTER_FAILURE;
    }

    // Find the correct register
    switch((int) broadcom_number / (REGISTER_SIZE / GPLEV_BITS_PER_PIN))
    {
    case 0:
        status_register = GPLEV0;
        break;
    case 1:
        status_register = GPLEV1;
        break;
    default:
        return REGISTER_FAILURE;
        break;
    }

    // Calculate the bit offset
    bit_offset = ((int) broadcom_number % (REGISTER_SIZE / GPLEV_BITS_PER_PIN)) * GPLEV_BITS_PER_PIN;

    // Get the pin value
    *pin_value = *((Register_Type*) gpio_memory + CALCULATE_OFFSET(status_register)) >> bit_offset;

    return SUCCESS;
}

static bool map_memory()
{
    int memory_file;

    // Attempt to open the memory file
    memory_file = open(MEMORY_FILE, O_RDWR);

    if(memory_file < 0)
    {
        return false;
    }

    // Attempt to map the memory
    gpio_memory = mmap(0x00, GPIO_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memory_file, GPIO_MEMORY_START);
    close(memory_file);

    if(gpio_memory == MAP_FAILED)
    {
        gpio_memory = 0x00;
        return false;
    }

    // Success once memory has been mapped
    return true;
}

static void unmap_memory()
{
    if(gpio_memory != 0)
    {
        munmap((void*) gpio_memory, GPIO_MEMORY_SIZE);
    }
}

static bool check_root()
{
    if(geteuid() == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static bool set_cpu()
{
    FILE* cpu_info_file;
    char cpu_info_stream[MAX_LINE_LENGTH];
    char chipset[MAX_LINE_LENGTH];
    int chipset_revision = -1;

    cpu_info_file = fopen(CPU_INFO_PATH, "r");

    if(!cpu_info_file)
    {
        return false;
    }

    while(fgets(cpu_info_stream, MAX_LINE_LENGTH, cpu_info_file))
    {
        sscanf(cpu_info_stream, CHIPSET_HEADER "%s", chipset);
        sscanf(cpu_info_stream, REVISION_HEADER "%x", &chipset_revision);
    }

    // Overvolted Raspberry Pi's are prefixed with 1000. Ignore it.
    chipset_revision %= REVISION_LENGTH;

    // Check chipset
    if(strcmp(CHIPSET, chipset) != 0)
    {
        return false;
    }

    // Check revision
    if(chipset_revision >= REVISION_1_START && chipset_revision < REVISION_2_START)
    {
        revision = 1;
    }
    else if(chipset_revision >= REVISION_2_START)
    {
        revision = 2;
    }
    else
    {
        return false;
    }

    return true;
}

static bool check_init()
{
    if(gpio_memory == 0x00)
    {
        return false;
    }

    if(revision == 0x00)
    {
        return false;
    }

    return true;
}

static bool pin_to_broadcom(int pin_number, PinType pin_type, int* broadcom_number)
{
    bool result;

    // Any pin type other than Broadcom needs to be checked as a physical pin
    if(pin_type != BROADCOM)
    {
        result = check_physical_pin(pin_number, pin_type, broadcom_number);

        if(!result)
        {
            return false;
        }
    }

    // Verify Broadcom number (or physical pin converted into a Broadcom)
    result = check_broadcom_pin(*broadcom_number);

    return result;
}

static bool check_broadcom_pin(int broadcom_number)
{
    if(broadcom_number <= GPIO_PIN_COUNT && broadcom_number >= 0)
    {
        return true;
    }

    return false;
}

static bool check_physical_pin(int physical_pin_number, PinType connector_type, int* broadcom_number)
{
	int table_size;
    const PhysicalPin* pin_table;
    int i;

    // Get the correct pin table based on the connector and revision
    if(connector_type == P1CONNECTOR)
    {
        if(revision == 1)
        {
            pin_table = REVISION_1_TABLE;
            table_size = sizeof(REVISION_1_TABLE) / sizeof(PhysicalPin);
        }
        else if(revision == 2)
        {
            pin_table = REVISION_2_TABLE;
            table_size = sizeof(REVISION_2_TABLE) / sizeof(PhysicalPin);
        }
        else
        {
            return false;
        }
    }

    /* Check the pin mapping tables to find the Broadcom GPIO pin number based on the 
	   type of numbering convention the user chose and the Raspberry Pi's revision.
	
	   Although a linear search is inefficient - O(N) complexity, the upper-bound to 
	   the physical number of pins is quite low (under 50) and fixed at compile time. 
	   Using a linear search on the pin tables prevents a very hard to track down 
	   defect: the programmer does not put pin mappings in a sequential order. */
    for(i = 0; i < table_size; i++)
    {
        /* If the physical pin number with a corresponding Broadcom pin number was 
		   found in the table, the user's pin number is valid. */
        if(physical_pin_number == pin_table[i].physical_pin_number)
        {
            *broadcom_number = pin_table[i].broadcom_pin_number;
			return true;
        }
    }

    return false;
}

static bool set_gpio_pin_function(int broadcom_number, int function_code)
{
    int function_register;
    int bit_offset;
    int i;

    // Function code must be 0 to 7
    if(function_code > 8 || function_code < 0)
    {
        return false;
    }

    // Set the correct function select register
    switch(broadcom_number / (REGISTER_SIZE / GPFSEL_BITS_PER_PIN))
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
        return false;
    }

    bit_offset = (broadcom_number % (REGISTER_SIZE / GPFSEL_BITS_PER_PIN)) * GPFSEL_BITS_PER_PIN;

    // The bits need to be cleared before they can be set again
    *((volatile int*) gpio_memory + CALCULATE_OFFSET(function_register)) &= ~(0x07 << bit_offset);

    // Now set the bits
    *((volatile int*) gpio_memory + CALCULATE_OFFSET(function_register)) |= function_code << bit_offset;

    return true;
}
