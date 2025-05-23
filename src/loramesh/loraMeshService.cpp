#include "loraMeshService.h"
#include "targets.h"
#include "common.h"
#include "logging.h"
static const char *LMS_TAG = "LoRaMeshService";

#if defined(NAYAD_V1) || defined(NAYAD_V1R2) || defined(T_BEAM_LORA_32) || defined(T_BEAM_V10) || defined(T_BEAM_V12)
SPIClass newSPI(HSPI);
#endif

bool LoRaMeshService::initLoraMesherService()
{
#ifdef LORA_ENABLE
    LoraMesher::LoraMesherConfig config = LoraMesher::LoraMesherConfig();

    config.loraCs = GPIO_PIN_NSS;
    config.loraRst = GPIO_PIN_RST;
    config.loraIrq = GPIO_PIN_BUSY;
    config.loraIo1 = GPIO_PIN_DIO1;

    ESP_LOGV(LMS_TAG, "LoraMesher config: CS: %d, RST: %d, IRQ: %d, IO1: %d", config.loraCs, config.loraRst, config.loraIrq, config.loraIo1);

#ifdef RADIO_SX128X
    config.module = LoraMesher::LoraModules::SX1280_MOD;
    config.freq = 2400.0F;
    config.bw = 812.5F;
    config.cr = 7U;
    config.sf = 7U;

#elif defined(RADIO_SX127X)
    config.module = LoraMesher::LoraModules::SX1276_MOD;
#elif defined(RADIO_SX126X)
    config.module = LoraMesher::LoraModules::SX1262_MOD;
#endif

#if defined(NAYAD_V1) || defined(NAYAD_V1R2) || defined(T_BEAM_LORA_32) || defined(T_BEAM_V10) || defined(T_BEAM_V12)
    newSPI.begin(GPIO_PIN_SCK, GPIO_PIN_MISO, GPIO_PIN_MOSI, GPIO_PIN_NSS);
    config.spi = &newSPI;
#else
    SPI.begin(GPIO_PIN_SCK, GPIO_PIN_MISO, GPIO_PIN_MOSI, GPIO_PIN_NSS);
    config.spi = &SPI;
#endif

    ESP_LOGV(LMS_TAG, "LoraMesher config: Module: %d", config.module);
    ESP_LOGV(LMS_TAG, "LoraMesher config: LORA_SCK: %d, LORA_MISO: %d, LORA_MOSI: %d, LORA_CS: %d", GPIO_PIN_SCK, GPIO_PIN_MISO, GPIO_PIN_MOSI, GPIO_PIN_NSS);

    // Initialize LoRaMesher
    // radio.begin(config);
    // radio.radio->getRSSI(GPIO_PIN_RX_ENABLE,GPIO_PIN_TX_ENABLE);
    // radio.radio->setRfSwitchPins(GPIO_PIN_RX_ENABLE,GPIO_PIN_TX_ENABLE);
    // radio.radio->setRfSwitchPins(GPIO_PIN_RX_ENABLE, GPIO_PIN_TX_ENABLE);
    radio.begin(config);

    // Create the receive task and add it to the LoRaMesher
    createReceiveMessages();

    // Start LoRaMesher
    radio.start();

    ESP_LOGV(LMS_TAG, "LoraMesher initialized");
    setConnectionState(connected);
    return 1;
#endif
}

void LoRaMeshService::loopReceivedPackets()
{
    // Iterate through all the packets inside the Received User Packets FiFo
    while (radio.getReceivedQueueSize() > 0)
    {
        ESP_LOGV(LMS_TAG, "LoRaPacket received");
        ESP_LOGV(LMS_TAG, "Queue receiveUserData size: %d", radio.getReceivedQueueSize());
        ESP_LOGV(LMS_TAG, "Heap size receive: %d", ESP.getFreeHeap());

        // Get the first element inside the Received User Packets FiFo
        AppPacket<LoRaMeshMessage> *packet = radio.getNextAppPacket<LoRaMeshMessage>();
        DBGLN("Reciver Lora: %c", packet->payload->dataMessage);
        // Create a DataMessage from the received packet
        DataMessage *message = createDataMessage(packet);

        // Process the packet
        MessageManager::getInstance().processReceivedMessage(LoRaMeshPort, message);

        // Delete the message
        vPortFree(message);

        // Delete the packet when used. It is very important to call this function to release the memory of the packet.
        radio.deletePacket(packet);
        ESP_LOGV(LMS_TAG, "Heap size receive2: %d", ESP.getFreeHeap());
    }
}

/**
 * @brief Function that process the received packets
 *
 */
void processReceivedPackets(void *)
{
    for (;;)
    {
        ESP_LOGV(LMS_TAG, "Stack space unused after entering the task: %d", uxTaskGetStackHighWaterMark(NULL));

        /* Wait for the notification of processReceivedPackets and enter blocking */
        ulTaskNotifyTake(pdPASS, portMAX_DELAY);
        LoRaMeshService::getInstance().loopReceivedPackets();
    }
}

/**
 * @brief Create a Receive Messages Task and add it to the LoRaMesher
 *
 */
void LoRaMeshService::createReceiveMessages()
{
    int res = xTaskCreate(
        processReceivedPackets,
        "Receive App Task",
        5000,
        (void *)1,
        2,
        &receiveLoRaMessage_Handle);
    if (res != pdPASS)
    {
        ESP_LOGE(LMS_TAG, "Receive App Task creation gave error: %d", res);
    }

    radio.setReceiveAppDataTaskHandle(receiveLoRaMessage_Handle);
}

LoRaMeshMessage *LoRaMeshService::createLoRaMeshMessage(DataMessage *message)
{
    LoRaMeshMessage *loraMeshMessage = (LoRaMeshMessage *)pvPortMalloc(sizeof(LoRaMeshMessage) + message->messageSize);

    if (loraMeshMessage)
    {
        loraMeshMessage->appPortDst = message->appPortDst;
        loraMeshMessage->appPortSrc = message->appPortSrc;
        loraMeshMessage->messageId = message->messageId;
        memcpy(loraMeshMessage->dataMessage, message->message, message->messageSize);
    }

    return loraMeshMessage;
}

DataMessage *LoRaMeshService::createDataMessage(AppPacket<LoRaMeshMessage> *appPacket)
{
    uint32_t dataMessageSize = appPacket->payloadSize + sizeof(DataMessage) - sizeof(LoRaMeshMessage);
    uint32_t messageSize = dataMessageSize - sizeof(DataMessage);

    DataMessage *dataMessage = (DataMessage *)pvPortMalloc(dataMessageSize);

    if (dataMessage)
    {
        LoRaMeshMessage *message = appPacket->payload;

        dataMessage->appPortDst = message->appPortDst;
        dataMessage->appPortSrc = message->appPortSrc;
        dataMessage->messageId = message->messageId;

        dataMessage->addrSrc = appPacket->src;
        dataMessage->addrDst = appPacket->dst;

        dataMessage->messageSize = messageSize;

        memcpy(dataMessage->message, message->dataMessage, messageSize);
    }

    return dataMessage;
}

uint16_t LoRaMeshService::getLocalAddress()
{
    return radio.getLocalAddress();
}

String LoRaMeshService::getRoutingTable()
{
    String routingTable = "--- Routing Table ---\n";

    // Set the routing table list that is being used and cannot be accessed (Remember to release use after usage)
    LM_LinkedList<RouteNode> *routingTableList = radio.routingTableListCopy();

    routingTableList->setInUse();

    if (routingTableList->moveToStart())
    {
        do
        {
            RouteNode *routeNode = routingTableList->getCurrent();
            NetworkNode node = routeNode->networkNode;
            String addr = String(node.address, HEX);
            addr.toUpperCase();
            String via = String(routeNode->via, HEX);
            via.toUpperCase();

            routingTable += addr + " (" + String(node.metric) + ") - Via: " + via + "\n";
        } while (routingTableList->next());
    }
    else
    {
        routingTable += "No routes";
    }

    // Release routing table list usage.
    routingTableList->releaseInUse();

    routingTableList->Clear();

    return routingTable;
}

void LoRaMeshService::send(DataMessage *message)
{
    ESP_LOGV(LMS_TAG, "Heap size send: %d", ESP.getFreeHeap());

    LoRaMeshMessage *loraMeshMessage = createLoRaMeshMessage(message);

#if SEND_RELIABLE == 0
    radio.createPacketAndSend(message->addrDst, (uint8_t *)loraMeshMessage, sizeof(LoRaMeshMessage) + message->messageSize);
#else
    radio.sendReliablePacket(message->addrDst, (uint8_t *)loraMeshMessage, sizeof(LoRaMeshMessage) + message->messageSize);
#endif

    vPortFree(loraMeshMessage);
    ESP_LOGV(LMS_TAG, "Heap size send 2: %d", ESP.getFreeHeap());
}

bool LoRaMeshService::sendClosestGateway(DataMessage *message)
{
    RouteNode *gatewayNode = radio.getClosestGateway();

    if (!gatewayNode)
    {
        ESP_LOGE(LMS_TAG, "No gateway found");
        return false;
    }

    message->addrDst = gatewayNode->networkNode.address;

    ESP_LOGI(LMS_TAG, "Sending message to gateway %X", message->addrDst);

    send(message);

    return true;
}

bool LoRaMeshService::hasActiveConnections()
{
    return radio.hasActiveConnections();
}

bool LoRaMeshService::hasActiveSentConnections()
{
    return radio.hasActiveSentConnections();
}

bool LoRaMeshService::hasActiveReceivedConnections()
{
    return radio.hasActiveReceivedConnections();
}

void LoRaMeshService::standby()
{
    return radio.standby();
}

bool LoRaMeshService::hasGateway()
{
    RouteNode *gatewayNode = radio.getClosestGateway();

    return gatewayNode != nullptr;
}

void LoRaMeshService::updateRoutingTable()
{
    if (radio.routingTableSize() == 0)
    {
        ESP_LOGW(LMS_TAG, "No routes in the routing table");
    }

    if (routingTableList != NULL)
    {
        routingTableList->Clear();
        delete (routingTableList);
    }

    routingTableList = radio.routingTableListCopy();
}
