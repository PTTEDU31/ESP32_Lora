#include "common.h"

#include "devLED.h"
device_affinity_t ui_devices[] = {
#ifdef HAS_RGB
    {&RGB_device, 0},
#endif
};
void setup()
{
    Serial.begin(115200);
#ifdef HAS_RGB
    Serial.print("HAS_RGB");
#endif
}
void loop() {}