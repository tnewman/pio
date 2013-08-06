/*
 * File:        gpio_utility.c
 * Description: Allows the user to get, set, and clear Raspberry Pi GPIO pins from the command line.
 * Programmer:  Aug 1, 2013
 * Date:        tnewman
 */

#include "../GPIO Driver/gpio.h"
#include <unistd.h>
#include <stdio.h>

void print_status_text(StatusCode code);

int main()
{
    StatusCode status;

    // Attempt to map memory
    status = map_gpio_memory();

    if(status != SUCCESS)
    {
        print_status_text(status);
    }

    // Drop setuid permissions
    seteuid(getuid());

    // Attempt to set pin
    status = set_pin(17, BROADCOM);
    print_status_text(status);

    // Unmap memory
    unmap_gpio_memory();

    return 0;
}

void print_status_text(StatusCode code)
{
    char* status_text;

    switch(code)
    {
    case SUCCESS:
        status_text = "GPIO Operation Complete";
        break;
    case NOT_ROOT:
        status_text = "GPIO Operation Failed - ID/Effective ID Not Root";
        break;
    case NO_CPU_INFO:
        status_text = "GPIO Operation Failed - Cannot Read System Information";
        break;
    case INVALID_CHIPSET:
        status_text = "GPIO Operation Failed - Invalid Chipset (might not be a Raspberry Pi)";
        break;
    case INVALID_REVISION:
        status_text = "GPIO Operation Failed - Invalid Revision";
        break;
    case CANNOT_MAP_MEMORY:
        status_text = "GPIO Operation Failed - Cannot Map GPIO Memory Region";
        break;
    case MEMORY_NOT_MAPPED:
        status_text = "GPIO Operation Failed - The GPIO Memory Region Is Not Mapped";
        break;
    case INVALID_TYPE:
        status_text = "GPIO Operation Failed - The Pin Type is Invalid (must be BROADCOM or P1)";
        break;
    case INVALID_PIN_NUMBER:
        status_text = "GPIO Operation Failed - The Pin Number is Not Valid";
        break;
    case INVALID_FUNCTION_CODE:
        status_text = "GPIO Operation Failed - The Function Code is Not Valid";
        break;
    default:
        status_text = "GPIO Operation Failed - Unknown Error";
        break;
    }

    printf("%s\n", status_text);
}
