#include <stdint.h>

#include "i2s.h"
#define MAX_EX_PIN 16

typedef struct pwm_pin
{
    uint32_t pwm_ticks = 0;
    uint32_t pwm_tick_count = 0;
    uint32_t pwm_duty_tick = 0;
}   pwm_pin;

class EX_HAL{
    public:
    static pwm_pin pwm_pin_data[MAX_EX_PIN];
    static void init_hal();
    static void set_pwm_duty(int pin,int duty = 255);
    static void set_pwm_frequency(int pin , uint32_t f);
};