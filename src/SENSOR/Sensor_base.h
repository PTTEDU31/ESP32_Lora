#pragma once

#include <ArduinoJson.h>

// Abstract Sensor Class
class Sensor {
public:

    virtual void initialize() = 0;
    // virtual void updateMeasurements() = 0;
    virtual void serialize(JsonObject& sensorObj) = 0;
    virtual ~Sensor() = default;
};
