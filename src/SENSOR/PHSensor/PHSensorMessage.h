#pragma once

#include <Arduino.h>

#include <ArduinoJson.h>

#pragma pack(1)
class PHSensorMessage {
public:
    float temperature;
    float ph;

    PHSensorMessage() {}

    PHSensorMessage(float temperature, float ph): temperature(temperature), ph(ph) {}

    void serialize(JsonArray& doc) {
        JsonObject tempObj = doc.add<JsonObject>();
        tempObj["measurement"] = temperature;
        tempObj["type"] = "Soil_Temperature";

        JsonObject phObj = doc.add<JsonObject>();
        phObj["measurement"] = ph;
        phObj["type"] = "Soil_PH";
    }
};
#pragma pack()