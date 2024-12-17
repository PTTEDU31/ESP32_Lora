#include "devSensor.h"

void 

device_t Sensor_dev = {
    .initialize = nullptr,
    .start = nullptr,
    .event  = nullptr,
    .timeout = nullptr,
}