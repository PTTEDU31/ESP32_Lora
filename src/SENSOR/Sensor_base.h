#pragma once

#include <ArduinoJson.h>

// Abstract Sensor Class
class Sensor_base
{
public:
    virtual void initialize() = 0;
    // virtual void updateMeasurements() = 0;
    virtual void serialize(JsonArray& doc) = 0;
};
