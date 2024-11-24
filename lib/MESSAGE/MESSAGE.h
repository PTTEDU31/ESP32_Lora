#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

#pragma pack(1)

// Message Ports
enum class messagePort : uint8_t {
    LoRaMeshPort = 1,
    BluetoothPort = 2,
    WiFiPort = 3,
    MqttPort = 4,
    InternalPort = 5,
};

// Application Ports
enum class appPort : uint8_t {
    LoRaChat = 1,
    BluetoothApp = 2,
    WiFiApp = 3,
    GPSApp = 4,
    WalletApp = 5,
    CommandApp = 6,
    LoRaMesherApp = 7,
    MQTTApp = 8,
    SimApp = 12,
    LedApp = 13,
    SensorApp = 14,
    MetadataApp = 15,
    MonApp = 16,
    DisplayApp = 17,
};

// Helper Functions for ArduinoJson and Enum
template <typename T>
T fromJson(JsonVariantConst value) {
    return static_cast<T>(value.as<uint8_t>());
}

template <typename T>
void toJson(T value, JsonVariant& json) {
    json = static_cast<uint8_t>(value);
}

// Base class
class DataMessageGeneric {
public:
    appPort appPortDst;
    appPort appPortSrc;
    uint8_t messageId;

    uint16_t addrSrc;
    uint16_t addrDst;

    uint32_t messageSize; // Message Size of the payload (excluding header)

    // Calculate total size
    uint32_t getDataMessageSize() const {
        return sizeof(DataMessageGeneric) + messageSize;
    }

    // Serialize to JSON
    void serialize(JsonObject& doc) const {
        // toJson(appPortDst, doc["appPortDst"]);
        // toJson(appPortSrc, doc["appPortSrc"]);
        doc["messageId"] = messageId;
        doc["addrSrc"] = addrSrc;
        doc["addrDst"] = addrDst;
        doc["messageSize"] = messageSize;
    }

    // Deserialize from JSON
    void deserialize(JsonObject& doc) {
        appPortDst = fromJson<appPort>(doc["appPortDst"]);
        appPortSrc = fromJson<appPort>(doc["appPortSrc"]);
        messageId = doc["messageId"].as<uint8_t>();
        addrSrc = doc["addrSrc"].as<uint16_t>();
        addrDst = doc["addrDst"].as<uint16_t>();
        messageSize = doc["messageSize"].as<uint32_t>();
    }
};

// Derived class for payload
class DataMessage : public DataMessageGeneric {
public:
    std::vector<uint8_t> message; // Use std::vector for dynamic array

    DataMessage(uint32_t size) : message(size) {
        messageSize = size;
    }

    // Serialize the message content
    void serialize(JsonObject& doc) const {
        DataMessageGeneric::serialize(doc); // Serialize base class members
        JsonArray dataArray = doc.createNestedArray("message");
        for (auto byte : message) {
            dataArray.add(byte);
        }
    }

    // Deserialize the message content
    void deserialize(JsonObject& doc) {
        DataMessageGeneric::deserialize(doc); // Deserialize base class members
        JsonArray dataArray = doc["message"];
        message.clear();
        for (auto value : dataArray) {
            message.push_back(value.as<uint8_t>());
        }
    }
};

#pragma pack()
