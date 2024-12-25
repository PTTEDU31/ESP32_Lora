#pragma once

#include "Sensor_base.h"
#include <Wire.h>
#include <vector>
#include <memory>
#include <ArduinoJson.h>

class SensorManager {
private:
    std::vector<std::shared_ptr<Sensor>> sensors;

    // Detect sensor by address
    std::shared_ptr<Sensor> detectSensor(uint8_t address);

public:
    void detectAndAddSensors(); // Automatically detect and add sensors
    void addSensor(std::shared_ptr<Sensor> sensor);
    void beginAll();
    // void updateAll();
    void serializeAll(JsonArray& sensorArray);
};