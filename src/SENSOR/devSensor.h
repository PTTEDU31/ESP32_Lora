#pragma once

#include "device.h"
#include "SensorService.h"
#include "common.h"
#include "device.h"
#if defined(USE_ANALOG_VBAT)
#include "devAnalogVbat.h"
#endif
enum eBaroReadState : uint8_t
{
    brsNoBaro,
    brsUninitialized,
    brsReadTemp,
    brsWaitingTemp,
    brsReadPres,
    brsWaitingPress
};

extern device_t Sensor_dev;

extern device_t Send_message;