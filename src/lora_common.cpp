#include "targets.h"
#include "common.h"
#include "config.h"
#include "logging.h"

#include <functional>


#if defined(USE_I2C)
#include <Wire.h>
#endif

boolean i2c_enabled = false;

static void setupWire()
{
#if defined(USE_I2C)
    int gpio_scl = GPIO_PIN_SCL;
    int gpio_sda = GPIO_PIN_SDA;

    if(gpio_sda != UNDEF_PIN && gpio_scl != UNDEF_PIN)
    {
        DBGLN("Starting wire on SCL %d, SDA %d", gpio_scl, gpio_sda);
        // ESP hopes to get Wire::begin(int, int)
        // ESP32 hopes to get Wire::begin(int = -1, int = -1, uint32 = 0)
        Wire.begin(gpio_sda, gpio_scl);
        Wire.setClock(400000);
        i2c_enabled = true;
    }
#endif
}

void setupTargetCommon()
{
    setupWire();
}