#include "gpio.h"
#include "register.h"

bool check_root();
bool check_chipset();
bool map_memory();
void unmap_memory();

StatusCode check_pin(int pin_number, PinType pin_type);
bool check_broadcom_pin(int broadcom_number);
bool check_physical_pin(int physical_number, PinType connector_type);
void set_gpio_pin_function(int broadcom_number);