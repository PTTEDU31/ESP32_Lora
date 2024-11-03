#include "lora_common.h"
#include "devLED.h"
device_affinity_t devices[] = {
    {&RGB_device, 1},
};

void setup() {
    Serial.begin(115200);
    devicesRegister(devices, ARRAY_SIZE(devices));  // Đăng ký 2 thiết bị
    devicesInit();
    devicesStart();
}

void loop() {
    unsigned long now = millis();
    devicesUpdate(now);
    // Serial.println(xPortGetCoreID());
}
