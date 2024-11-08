#include "lora_common.h"
#include "devLED.h"
#include "devWIFI.h"
device_affinity_t devices[] = {
    #ifdef HAS_RGB
    {&RGB_device, 1},
    #endif
    {&WIFI_device, 1}
};


bool setupHardwareFromOptions()
{
      if (!options_init())
  {
    // Register the WiFi with the framework
    static device_affinity_t wifi_device[] = {
        {&WIFI_device, 1}
    };
    devicesRegister(wifi_device, ARRAY_SIZE(wifi_device));
    devicesInit();

    connectionState = hardwareUndefined;
    return false;
  }
}
void setup() {
    Serial.begin(115200);
    
    devicesRegister(devices, ARRAY_SIZE(devices));  // Đăng ký 2 thiết bị
    devicesInit();
    devicesStart();
    read_flash_data();
}

void loop() {
    unsigned long now = millis();
    devicesUpdate(now);
    // Serial.println(xPortGetCoreID());
}
