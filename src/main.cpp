#include "HAL/include.h"
#include "HAL/HAL.h"
void setup() {
    Serial.begin(115200);
    hal.init_hal();
    hal.set_pwm_duty(129,200);
    hal.set_pwm(128,128,50);
}

void loop() {

}
