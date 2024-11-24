#include "logging.h"
enum messagePort: uint8_t {
    LoRaMeshPort = 1,
    BluetoothPort = 2,
    WiFiPort = 3,
    MqttPort = 4,
    InternalPort = 5,
    Rs485Port = 6
};
enum appPort: uint8_t {
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

class DataMessageGeneric {
public:
    appPort appPortDst;
    appPort appPortSrc;
    uint8_t messageId;

    uint16_t addrSrc;
    uint16_t addrDst;

    uint32_t messageSize; //Message Size of the payload no include header

    void deserialize(JsonObject &doc);
    void serialize(JsonObject &doc);
    uint32_t getDataMessageSize();
};

class DataMessage: public DataMessageGeneric {
public:
    uint8_t message[];
};
