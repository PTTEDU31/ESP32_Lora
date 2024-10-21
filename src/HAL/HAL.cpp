#include "include.h"
pwm_pin_t EX_HAL::pwm_pin_data[MAX_EX_PIN];

// get PWM channel for pin - if none then attach a new one
// return -1 if fail or invalid pin#, channel # (0-15) if success
int8_t get_pwm_channel(const pin_t pin, const uint32_t freq, const uint16_t res) {
  if (!WITHIN(pin, 1, MAX_PWM_IOPIN)) return -1; // Not a hardware PWM pin!
  int8_t cid = channel_for_pin(pin);
  if (cid >= 0) return cid;

  // Find an empty adjacent channel (same timer & freq/res)
  for (int i = 0; i <= CHANNEL_MAX_NUM; i++) {
    if (chan_pin[i] == 0) {
      if (chan_pin[i ^ 0x1] != 0) {
        if (pwmInfo[i / 2].freq == freq && pwmInfo[i / 2].res == res) {
          chan_pin[i] = pin; // Allocate PWM to this channel
          ledcAttachPin(pin, i);
          return i;
        }
      }
      else if (cid == -1)    // Pair of empty channels?
        cid = i & 0xFE;      // Save lower channel number
    }
  }
  // not attached, is an empty timer slot avail?
  if (cid >= 0) {
    chan_pin[cid] = pin;
    pwmInfo[cid / 2].freq = freq;
    pwmInfo[cid / 2].res = res;
    ledcSetup(cid, freq, res);
    ledcAttachPin(pin, cid);
  }
  return cid; // -1 if no channel avail
}

EX_HAL::EX_HAL() {
}

void EX_HAL::set_pwm_duty(int pin, int duty = 255) {
    if (pin > 127) {
        const uint8_t pinlo = pin & 0x7F;
        pwm_pin_t &pindata = pwm_pin_data[pinlo];
        
        // Tính toán duty cycle dựa trên giá trị v mà không cần invert
        const uint32_t duty = map(v, 0, v_size, 0, pindata.pwm_cycle_ticks);

        if (duty == 0 || duty == pindata.pwm_cycle_ticks) {  // max or min (on/off)
            pindata.pwm_duty_ticks = 0;                      // turn off PWM for this pin
            duty ? SBI32(i2s_port_data, pinlo) : CBI32(i2s_port_data, pinlo); // set pin level
        }
        else {
            pindata.pwm_duty_ticks = duty;  // PWM duty count = # of 4µs ticks per full PWM cycle
        }

        return;
    }

    const int8_t cid = get_pwm_channel(pin, PWM_FREQUENCY, PWM_RESOLUTION);
    if (cid >= 0) {
        // Tính toán duty cycle mà không cần invert
        const uint32_t duty = map(v, 0, v_size, 0, _BV(PWM_RESOLUTION) - 1);
        ledcWrite(cid, duty);
    }
}
