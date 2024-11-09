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

#if defined(TARGET_RX) && defined(GPIO_PIN_PWM_OUTPUTS)
    for (int ch = 0 ; ch < GPIO_PIN_PWM_OUTPUTS_COUNT ; ++ch)
    {
        auto pin = GPIO_PIN_PWM_OUTPUTS[ch];
        auto pwm = config.GetPwmChannel(ch);
        // if the PWM pin is nominated as SDA or SCL, and it's not configured for I2C then undef the pins
        if ((pin == GPIO_PIN_SCL && pwm->val.mode != somSCL) || (pin == GPIO_PIN_SDA && pwm->val.mode != somSDA))
        {
            gpio_scl = UNDEF_PIN;
            gpio_sda = UNDEF_PIN;
            break;
        }
        // If I2C pins are not defined in the hardware, then look for configured I2C
        if (GPIO_PIN_SCL == UNDEF_PIN && pwm->val.mode == somSCL)
        {
            gpio_scl = pin;
        }
        if (GPIO_PIN_SCL == UNDEF_PIN && pwm->val.mode == somSDA)
        {
            gpio_sda = pin;
        }
    }
#endif
    if(gpio_sda != UNDEF_PIN && gpio_scl != UNDEF_PIN)
    {
        // DBGLN("Starting wire on SCL %d, SDA %d", gpio_scl, gpio_sda);
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