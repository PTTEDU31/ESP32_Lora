#include "devSensor.h"
#include "targets.h"
#include "INA219.h"
#include "logging.h"
#include "Wire.h"
INA219 INA(0x40);

static void initialize(){
#ifdef  OPT_HAS_CURRENT
#ifdef OPT_HAS_CURRENT_INA219
    // devCurrent = new devINA219();
    if(!INA.begin())
        DBGLN("INA219 init false");
#endif

#endif
}
static int stat(){
    return DURATION_IMMEDIATELY;
}

static int timeout(){
    DBGLN("Current INA219 %f",INA.getBusVoltage_mV());
    return 10000;

}

device_t Sensor_dev = {
    .initialize = initialize,
    .start = stat,
    .event  = nullptr,
    .timeout = timeout
};