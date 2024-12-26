#include "SensorManager.h"
// #include "BMP280Sensor.h"

// std::shared_ptr<Sensor> SensorManager::detectSensor(uint8_t address) {
//     Wire.beginTransmission(address);
//     if (Wire.endTransmission() == 0) {
//         // Try identifying the sensor by testing specific registers
//         if (address == 0x40 || address == 0x41) { // INA319 example
//             return std::make_shared<INA319Sensor>(address);
//         }
//         else if (address == 0x76 || address == 0x77) { // BMP280 example
//             return std::make_shared<BMP280Sensor>(address);
//         }
//     }
//     return nullptr; // No sensor detected
// }

void SensorManager::detectAndAddSensors() {
    for (uint8_t address = 0x03; address <= 0x77; address++) {
        auto sensor = detectSensor(address);
        if (sensor) {
            sensors.push_back(sensor);
            Serial.printf("Sensor detected at 0x%02X\n", address);
        }
    }
}

void SensorManager::addSensor(std::shared_ptr<Sensor_base> sensor) {
    sensors.push_back(sensor);
}

void SensorManager::beginAll() {
    for (const auto& sensor : sensors) {
        sensor->initialize();
    }
}

// void SensorManager::updateAll() {
//     for (const auto& sensor : sensors) {
//         sensor->updateMeasurements();
//     }
// }

void SensorManager::serializeAll(JsonArray& sensorArray) {
    for (const auto& sensor : sensors) {
        sensor->serialize(sensorArray); // Gọi trực tiếp serialize từ từng cảm biến
    }
}
