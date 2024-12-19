#pragma once

#include "targets.h"

class CurrentSensor
{
public:
    virtual bool begin() = 0;
    // virtual bool isConnected() = 0;

    // virtual float getBusVoltage_mV() = 0;
    // virtual float getCurrent_mA() = 0;
    // virtual float getShuntVoltage_mV() = 0;
    // virtual float getPower_mW() = 0;
};