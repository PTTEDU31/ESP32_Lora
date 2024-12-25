#pragma once
#include "messageService.h"
#include "sensorCommandService.h"
class SensorService : public MessageService
{
public:
    static SensorService &getInstance()
    {
        static SensorService instance;
        return instance;
    }
    static bool initialize();
    String getJSON(DataMessage *message);
    DataMessage *getDataMessage(JsonObject data);
    // void processReceivedMessage(messagePort port, DataMessage *message);

    SensorCommandService *sensorCommandService = new SensorCommandService();

private:
    SensorService() : MessageService(SensorApp, "Sensor")
    {
        commandService = sensorCommandService;
    };
};