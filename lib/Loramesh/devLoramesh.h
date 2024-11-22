#pragma once

#include "LoraMesher.h"

class LoRaMeshService
{
public:
    static LoRaMeshService &getInstance()
    {
        static LoRaMeshService instance;
        return instance;
    }
    void initLoraMeshService();
    void loopReceivedPackets();

private:
    LoraMesher &radio = LoraMesher::getInstance();
    void createReceiveMessages();
    TaskHandle_t receiveLoRaMessage_Handle = NULL;
};