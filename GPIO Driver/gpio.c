#include "gpio.h"
#include "register.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdbool.h>

volatile Register_Type* gpio_memory = 0x00;
int revision;

static bool map_memory();
static bool check_root();
static bool check_chipset();
static void unmap_memory();
static bool check_pin(int pin_number, PinType pin_type);
static bool check_broadcom_pin(int broadcom_number);
static bool check_physical_pin(int physical_number, PinType connector_type);
static void set_gpio_pin_function(int broadcom_number);

static bool map_memory()
{
    int memory_file;

    // Attempt to open the memory file
    memory_file = open(MEMORY_FILE, O_RDONLY);

    if(memory_file < 0)
    {
        return false;
    }

    // Attempt to map the memory
    gpio_memory = mmap(0x00, GPIO_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memory_file, GPIO_MEMORY_START);
    close(memory_file);

    if(gpio_memory == 0)
    {
        return false;
    }

    // Success once memory has been mapped
    return true;
}

static void unmap_memory()
{
    if(gpio_memory != 0)
    {
        munmap(gpio_memory, GPIO_MEMORY_SIZE);
    }
}

static bool check_root()
{
    if(geteuid() != 0)
    {
        return SUCCESS;
    }
    else
    {
        return NOT_ROOT;
    }
}

static bool check_chipset()
{
    FILE* cpu_info_file;
    char cpu_info_stream[MAX_LINE_LENGTH];
    char chipset[MAX_LINE_LENGTH];
    int chipset_revision = -1;

    cpu_info_file = fopen(CPU_INFO_PATH, "r");

    if(!cpu_info_file)
    {
        return NO_CPU_INFO;
    }

    while(fread(cpu_info_stream, MAX_LINE_LENGTH - 1, 1, cpu_info_file))
    {
        sscanf(cpu_info_stream, CHIPSET_HEADER "%s", chipset);
        sscanf(cpu_info_stream, REVISION_HEADER "%d", chipset_revision);
    }

    // Overvolted Raspberry Pi's are prefixed with 1000. Ignore it.
    chipset_revision %= REVISION_LENGTH;

    // Check chipset
    if(strcmp(CHIPSET, chipset) != 0)
    {
        return INVALID_CHIPSET;
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
        return INVALID_REVISION;
    }

    return SUCCESS;
}
