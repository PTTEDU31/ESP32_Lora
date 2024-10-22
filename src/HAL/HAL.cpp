#include "include.h"
// use 12 bit precision for LEDC timer
#define LEDC_TIMER_12_BIT 12

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ 1000

// define starting duty, target duty and maximum fade time
#define LEDC_START_DUTY  (0)
#define LEDC_TARGET_DUTY (4095)

pwm_pin_t EX_HAL::pwm_pin_data[MAX_EX_PIN];
EX_HAL::EX_HAL() {
}
void EX_HAL::set_pwm_duty(int pin, int duty) {
    if (pin > 127) {
        const uint8_t pinlo = pin & 0x7F;
        pwm_pin_t &pindata = pwm_pin_data[pinlo];
        const uint32_t duty = map(duty, 0, 255, 0, pindata.pwm_cycle_ticks);

        if (duty == 0 || duty == pindata.pwm_cycle_ticks) {  // max or min (on/off)
            pindata.pwm_duty_ticks = 0;                      // turn off PWM for this pin
            duty ? SBI32(i2s_port_data, pinlo) : CBI32(i2s_port_data, pinlo); // set pin level
        }
        else {
            pindata.pwm_duty_ticks = duty;  // PWM duty count = # of 4µs ticks per full PWM cycle
        }

        return;
    }

    ledcWrite(pin,map(duty,0,255,0,LEDC_TARGET_DUTY));
}

void EX_HAL::set_pwm_frequency(int pin, uint16_t f){
  if(pin > 127){
      pwm_pin_data[pin & 0x7F].pwm_cycle_ticks = 1000000UL / f / 4; // # of 4µs ticks per full PWM cycle
      return ;
  }
  ledcChangeFrequency(pin,f,LEDC_TIMER_12_BIT);

}