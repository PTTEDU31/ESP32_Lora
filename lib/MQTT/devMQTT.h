#pragma once

#include "device.h"
#include "mqttCommandService.h"
#include "messageService.h"

#include "messageManager.h"
class DEV_MQTT : public MessageService
{
public:
    static DEV_MQTT &getInstance()
    {
        static DEV_MQTT instance;
        return instance;
    }
    void initMqtt(String localName);

    bool isInitialized()
    {
        return initialized;
    }

    static void connectToMqtt();
    static bool writeToMqtt(String message);
    static bool writeToMqtt(DataMessage *message);
    bool isMQTTConnected();

    void processReceivedMessageFromMQTT(String &topic, String &payload);

    MqttCommandService *mqttCommandService = new MqttCommandService();

private:
    DEV_MQTT() : MessageService(appPort::MQTTApp, String("MQTT"))
    {
        commandService = mqttCommandService;
    };
    bool initialized = false;
};

extern device_t MQTT_device;