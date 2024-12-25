#pragma once
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
    static bool initialize();

    bool isInitialized()
    {
        return initialized;
    }

    static void connectToMqtt();
    static bool writeToMqtt(String message);
    static bool writeToMqtt(DataMessage *message);
    bool isMQTTConnected();
    virtual void processReceivedMessage(messagePort port, DataMessage *message);
    MqttCommandService *mqttCommandService = new MqttCommandService();

private:
    DEV_MQTT() : MessageService(appPort::MQTTApp, String("MQTT"))
    {
        commandService = mqttCommandService;
    };
    static bool initialized; // Khai báo tĩnh
};