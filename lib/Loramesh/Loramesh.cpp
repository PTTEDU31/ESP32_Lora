#include "Arduino.h"
#include "targets.h"
#include "devLoramesh.h"
static const char *LMS_TAG = "LoRaMeshService";

void LoRaMeshService::loopReceivedPackets() {
    //Iterate through all the packets inside the Received User Packets FiFo
    while (radio.getReceivedQueueSize() > 0) {
        ESP_LOGV(LMS_TAG, "LoRaPacket received");
        ESP_LOGV(LMS_TAG, "Queue receiveUserData size: %d", radio.getReceivedQueueSize());
        ESP_LOGV(LMS_TAG, "Heap size receive: %d", ESP.getFreeHeap());

        // //Get the first element inside the Received User Packets FiFo
        // AppPacket<LoRaMeshMessage>* packet = radio.getNextAppPacket<LoRaMeshMessage>();

        // //Create a DataMessage from the received packet
        // DataMessage* message = createDataMessage(packet);

        //Process the packet
        // MessageManager::getInstance().processReceivedMessage(LoRaMeshPort, message);

        //Delete the message
        // vPortFree(message);

        //Delete the packet when used. It is very important to call this function to release the memory of the packet.
        // radio.deletePacket(packet);
        ESP_LOGV(LMS_TAG, "Heap size receive2: %d", ESP.getFreeHeap());
    }
}

void processReceivedPackets(void*) {
    for (;;) {
        ESP_LOGV(LMS_TAG, "Stack space unused after entering the task: %d", uxTaskGetStackHighWaterMark(NULL));

        /* Wait for the notification of processReceivedPackets and enter blocking */
        ulTaskNotifyTake(pdPASS, portMAX_DELAY);
        LoRaMeshService::getInstance().loopReceivedPackets();
    }
}

void LoRaMeshService::createReceiveMessages() {
    int res = xTaskCreate(
        processReceivedPackets,
        "Receive App Task",
        5000,
        (void*) 1,
        2,
        &receiveLoRaMessage_Handle);
    if (res != pdPASS) {
        ESP_LOGE(LMS_TAG, "Receive App Task creation gave error: %d", res);
    }

    radio.setReceiveAppDataTaskHandle(receiveLoRaMessage_Handle);
}

void LoRaMeshService::initLoraMeshService()
{
#ifdef LORA_ENABLE
    LoraMesher::LoraMesherConfig config = LoraMesher::LoraMesherConfig();

    config.loraCs = GPIO_PIN_NSS;
    config.loraIo1 = GPIO_PIN_DIO1;
    config.loraRst = GPIO_PIN_RST;
    config.loraIrq = GPIO_PIN_BUSY;

    ESP_LOGV(LMS_TAG, "LoraMesher config: CS: %d, RST: %d, IRQ: %d, IO1: %d", config.loraCs, config.loraRst, config.loraIrq, config.loraIo1);

#if define(RADIO_SX128X)
    config.module = LoraMesher::LoraModules::SX1280_MOD;
#elif define(RADIO_SX127X)
    config.module = LoraMesher::LoraModules::SX1278_MOD;
#endif

    SPI.begin(GPIO_PIN_NSS, GPIO_PIN_MISO, GPIO_PIN_MOSI, -1);
    config.spi = &SPI;
    ESP_LOGV(LMS_TAG, "LoraMesher config: Module: %d", config.module);
    ESP_LOGV(LMS_TAG, "LoraMesher config: LORA_SCK: %d, LORA_MISO: %d, LORA_MOSI: %d, LORA_CS: %d", GPIO_PIN_SCK, GPIO_PIN_MISO, GPIO_PIN_MOSI, GPIO_PIN_NSS);
     //Initialize LoRaMesher
    radio.begin(config);
        //Create the receive task and add it to the LoRaMesher
    createReceiveMessages();

    //Start LoRaMesher
    radio.start();

    ESP_LOGV(LMS_TAG, "LoraMesher initialized");
#endif
}