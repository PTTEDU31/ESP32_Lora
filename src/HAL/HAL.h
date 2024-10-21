#pragma once
#include <stdint.h>
#include "../Config.h"
typedef struct pwm_pin
{
    uint32_t pwm_ticks = 0;
    uint32_t pwm_tick_count = 0;
    uint32_t pwm_duty_ticks = 0;
}   pwm_pin_t;

class EX_HAL{
    public:
    EX_HAL();
    static pwm_pin_t pwm_pin_data[MAX_EX_PIN];
    static void init_hal();
    static void set_pwm_duty(int pin,int duty = 255);
    static void set_pwm_frequency(int pin , uint32_t f);
};