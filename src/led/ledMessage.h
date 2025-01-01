#pragma once

#include <Arduino.h>

#include "dataMessage.h"

#pragma pack(1)

enum LedCommand : uint8_t
{
    Off = 0,
    On = 1,
    Sendata = 2,
    reciver = 3
};
class PWMCommandMessage : public DataMessageGeneric
{
public:
    LedCommand  ledCommand; // Lệnh LED (On/Off)
    uint8_t duty;          // Độ sáng (PWM duty cycle)
    uint32_t frequency;    // Tần số PWM

    // Hàm serialize: chuyển dữ liệu thành JSON
    void serialize(JsonObject &doc)
    {
        // Gọi hàm serialize của lớp cha
        DataMessageGeneric::serialize(doc);

        // Tạo đối tượng JSON lồng
        JsonObject dataObj = doc.createNestedObject("data");

        // Thêm dữ liệu vào JSON
        dataObj["ledCommand"] = static_cast<uint8_t>(ledCommand);
        dataObj["duty"] = duty;
        dataObj["frequency"] = frequency;
    }

    // Hàm deserialize: đọc dữ liệu từ JSON
    void deserialize(JsonObject &doc)
    {
        // Gọi hàm deserialize của lớp cha
        DataMessageGeneric::deserialize(doc);

        // Lấy dữ liệu từ JSON
        if (doc.containsKey("data"))
        {
            JsonObject dataObj = doc["data"];
            ledCommand = static_cast<LedCommand>(dataObj["ledCommand"].as<uint8_t>());
            duty = dataObj["duty"];
            frequency = dataObj["frequency"];
        }
    }
};
class LedMessage : public DataMessageGeneric
{
public:
    LedCommand ledCommand;
    uint8_t payload[];
    void serialize(JsonObject &doc)
    {
        switch (ledCommand)
        {
        case LedCommand::On:
        case LedCommand::Off:
            // Call the base class serialize function
            ((DataMessageGeneric *)(this))->serialize(doc);
            doc["ledCommand"] = ledCommand;
            break;
        case LedCommand::Sendata:
        case LedCommand::reciver:
            ((PWMCommandMessage *)(this))->serialize(doc);
            break;
        }
    }
    void deserialize(JsonObject &doc)
    {
        // Call the base class deserialize function
        switch ((LedCommand)doc["ledCommand"])
        {
        case LedCommand::On:
        case LedCommand::Off:
            break;
        case LedCommand::Sendata:
        case LedCommand::reciver:
            ((DataMessageGeneric *)(this))->deserialize(doc);
            break;
            // Add the derived class data to the JSON object
        }
        ledCommand = doc["ledCommand"];
    }
};

#pragma pack()