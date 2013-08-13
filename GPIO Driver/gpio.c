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

volatile Register_Type* gpio_memory = 0x00;
int revision = 0x00;

/*
 * Physical Pin Tables
 *
 * These tables map physical pin numbers for each supported revision to the actual pin
 * numbers used internally by Broadcom. The physical pin numbers must be in numerical
 * order for the binary search to work properly. This is on the programmer to get right.
 */
const PhysicalPin REVISION_1_TABLE[] = {{3, 0}, {5, 1}, {7, 4}, {8, 14}, {10, 15}, {11, 17}, {12, 18},
        {13, 21}, {15, 22}, {16, 23}, {18, 24}, {19, 10}, {21, 9}, {22, 25}, {23, 11}, {24, 8}, {26, 7}};
const PhysicalPin REVISION_2_TABLE[] = {{3, 2}, {5, 3}, {7, 4}, {8, 14}, {10, 15}, {11, 17}, {12, 18},
        {13, 27}, {15, 22}, {16, 23}, {18, 24}, {19, 10}, {21, 9}, {22, 25}, {23, 11}, {24, 8}, {26, 7}};

/*
 * Implementation Functions
 */
static bool map_memory();
static void unmap_memory();
static bool check_root();
static bool set_cpu();
static bool check_init();
static bool pin_to_broadcom(int* pin_number, PinType pin_type);
static bool check_broadcom_pin(int broadcom_number);
static bool check_physical_pin(int physical_number, PinType connector_type, int* broadcom_number);
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

    // Check initialization
    if(!check_init())
    {
        return NO_INIT;
    }

    // Attempt to get the Broadcom pin number
    status = pin_to_broadcom(&pin_number, pin_type);

    if(!status)
    {
        return INVALID_PIN;
    }

    // Set the pin function to output mode
    status = set_gpio_pin_function(pin_number, GPIO_OUTPUT);

    if(!status)
    {
        return REGISTER_FAILURE;
    }

    // Find the correct register
    switch((int) pin_number / (REGISTER_SIZE / GPSET_BITS_PER_PIN))
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
    bit_offset = ((int) pin_number % (REGISTER_SIZE / GPSET_BITS_PER_PIN)) * GPSET_BITS_PER_PIN;
    // Set the pin
    *((Register_Type*) gpio_memory + CALCULATE_OFFSET(set_register)) = GPSET_BITS << bit_offset;

    return SUCCESS;
}

StatusCode clear_gpio_pin(int pin_number, PinType pin_type)
{
    bool status;
    int clear_register;
    int bit_offset;

    // Check initialization
    if(!check_init())
    {
        return NO_INIT;
    }

    // Attempt to get the Broadcom pin number
    status = pin_to_broadcom(&pin_number, pin_type);

    if(!status)
    {
        return INVALID_PIN;
    }

    // Set the pin function to output mode
    status = set_gpio_pin_function(pin_number, GPIO_OUTPUT);

    if(!status)
    {
        return REGISTER_FAILURE;
    }

    // Find the correct register
    switch((int) pin_number / (REGISTER_SIZE / GPCLR_BITS_PER_PIN))
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
    bit_offset = ((int) pin_number % (REGISTER_SIZE / GPCLR_BITS_PER_PIN)) * GPCLR_BITS_PER_PIN;

    // Clear the pin
    *((Register_Type*) gpio_memory + CALCULATE_OFFSET(clear_register)) = GPCLR_BITS << bit_offset;

    return SUCCESS;
}

StatusCode get_gpio_pin(int pin_number, PinType pin_type, int* pin_value)
{
    bool status;
    int status_register;
    int bit_offset;

    // Check initialization
    if(!check_init())
    {
        return NO_INIT;
    }

    // Attempt to get the Broadcom pin number
    status = pin_to_broadcom(&pin_number, pin_type);

    if(!status)
    {
        return INVALID_PIN;
    }

    // Set the pin function to input mode
    status = set_gpio_pin_function(pin_number, GPIO_INPUT);

    if(!status)
    {
        return REGISTER_FAILURE;
    }

    // Find the correct register
    switch((int) pin_number / (REGISTER_SIZE / GPLEV_BITS_PER_PIN))
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
    bit_offset = ((int) pin_number % (REGISTER_SIZE / GPLEV_BITS_PER_PIN)) * GPLEV_BITS_PER_PIN;

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

static bool pin_to_broadcom(int* pin_number, PinType pin_type)
{
    bool result;

    // Any pin type other than Broadcom needs to be checked as a physical pin
    if(pin_type != BROADCOM)
    {
        result = check_physical_pin(*pin_number, pin_type, pin_number);

        if(!result)
        {
            return false;
        }
    }

    // Verify Broadcom number (or physical pin converted into a Broadcom)
    result = check_broadcom_pin(*pin_number);

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

static bool check_physical_pin(int physical_number, PinType connector_type, int* broadcom_number)
{
    int left_bound;
    int right_bound;
    int middle;
    const PhysicalPin* pin_table;

    // Get the correct pin table based on the connector and revision
    if(connector_type == P1CONNECTOR)
    {
        if(revision == 1)
        {
            pin_table = REVISION_1_TABLE;
            left_bound = 0;
            right_bound = sizeof(REVISION_1_TABLE) / sizeof(PhysicalPin);
        }
        else if(revision == 2)
        {
            pin_table = REVISION_2_TABLE;
            left_bound = 0;
            right_bound = sizeof(REVISION_2_TABLE) / sizeof(PhysicalPin);
        }
        else
        {
            return false;
        }
    }

    /*
     * If the bounds pass each other, the whole thing has been checked, and
     * the number was not there.
     */
    while(left_bound <= right_bound)
    {
        middle = (right_bound - left_bound) / 2;

        // The pin is in the lower half if it is smaller than the middle
        if(physical_number < pin_table[middle].physical_pin_number)
        {
            right_bound = middle - 1;
        }
        // The pin is in the upper half if it is larger than the middle
        else if(physical_number > pin_table[middle].physical_pin_number)
        {
            left_bound = middle + 1;
        }
        // The pin exists in the array
        else
        {
            *broadcom_number = pin_table[middle].broadcom_pin_number;
            return true;
        }
    }

    return false;
}

static bool set_gpio_pin_function(int broadcom_number, int function_code)
{
    int function_register;
    int bit_offset;

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
