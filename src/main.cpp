#include <Arduino.h>
#include "LoraMesher.h"
#include "common.h"

// Using LILYGO TTGO T-BEAM v1.1
#define BOARD_LED 4
#define LED_ON LOW
#define LED_OFF HIGH

LoraMesher &radio = LoraMesher::getInstance();

uint32_t dataCounter = 0;
struct dataPacket
{
    uint32_t counter = 0;
};

dataPacket *helloPacket = new dataPacket;

// Led flash
void led_Flash(uint16_t flashes, uint16_t delaymS)
{
    uint16_t index;
    for (index = 1; index <= flashes; index++)
    {
        digitalWrite(BOARD_LED, LED_ON);
        delay(delaymS);
        digitalWrite(BOARD_LED, LED_OFF);
        delay(delaymS);
    }
}

/**
 * @brief Print the counter of the packet
 *
 * @param data
 */
void printPacket(dataPacket data)
{
    Serial.printf("Hello Counter received nº %d\n", data.counter);
}

/**
 * @brief Iterate through the payload of the packet and print the counter of the packet
 *
 * @param packet
 */
void printDataPacket(AppPacket<dataPacket> *packet)
{
    Serial.printf("Packet arrived from %X with size %d\n", packet->src, packet->payloadSize);

    // Get the payload to iterate through it
    dataPacket *dPacket = packet->payload;
    size_t payloadLength = packet->getPayloadLength();

    for (size_t i = 0; i < payloadLength; i++)
    {
        // Print the packet
        printPacket(dPacket[i]);
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
        /* Wait for the notification of processReceivedPackets and enter blocking */
        ulTaskNotifyTake(pdPASS, portMAX_DELAY);
        led_Flash(1, 100); // one quick LED flashes to indicate a packet has arrived

        // Iterate through all the packets inside the Received User Packets Queue
        while (radio.getReceivedQueueSize() > 0)
        {
            Serial.println("ReceivedUserData_TaskHandle notify received");
            Serial.printf("Queue receiveUserData size: %d\n", radio.getReceivedQueueSize());

            // Get the first element inside the Received User Packets Queue
            AppPacket<dataPacket> *packet = radio.getNextAppPacket<dataPacket>();

            // Print the data packet
            printDataPacket(packet);

            // Delete the packet when used. It is very important to call this function to release the memory of the packet.
            radio.deletePacket(packet);
        }
    }
}

TaskHandle_t receiveLoRaMessage_Handle = NULL;

/**
 * @brief Create a Receive Messages Task and add it to the LoRaMesher
 *
 */
void createReceiveMessages()
{
    int res = xTaskCreate(
        processReceivedPackets,
        "Receive App Task",
        4096,
        (void *)1,
        2,
        &receiveLoRaMessage_Handle);
    if (res != pdPASS)
    {
        Serial.printf("Error: Receive App Task creation gave error: %d\n", res);
    }

    radio.setReceiveAppDataTaskHandle(receiveLoRaMessage_Handle);
}

/**
 * @brief Initialize LoRaMesher
 *
 */
void setupLoraMesher()
{
    // Example on how to change the module. See LoraMesherConfig to see all the configurable parameters.
    LoraMesher::LoraMesherConfig config;
    config.module = LoraMesher::LoraModules::SX1262_MOD;

    // Init the loramesher with a processReceivedPackets function
    radio.begin(config);

    // Create the receive task and add it to the LoRaMesher
    createReceiveMessages();

    // Start LoRaMesher
    radio.start();

    Serial.println("Lora initialized");
}
bool setupHardwareFromOptions()
{

  if (!options_init())
  {
    // Register the WiFi with the framework
//     static device_affinity_t wifi_device[] = {
//         {&WIFI_device, 1}
//     };
//     devicesRegister(wifi_device, ARRAY_SIZE(wifi_device));
//     devicesInit();

//     connectionState = hardwareUndefined;
    return false;
  }
  return true;
}
#include "SPIFFS.h"

void setup() {
    Serial.begin(115200);
    
    // Khởi tạo SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount file system");
        return;
    }
    
    // Nếu gắn kết thành công
    Serial.println("SPIFFS mounted successfully");

    // Tạo một tệp và ghi dữ liệu vào
    File file = SPIFFS.open("/test.txt", FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    file.println("Hello, SPIFFS!");
    file.close();

    // Đọc dữ liệu từ tệp
    file = SPIFFS.open("/test.txt", FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }
    Serial.println("File content:");
    while (file.available()) {
        Serial.write(file.read());
    }
    file.close();
}



void loop()
{
    // for (;;)
    // {
    //     Serial.printf("Send packet %d\n", dataCounter);

    //     helloPacket->counter = dataCounter++;

    //     // Create packet and send it.
    //     radio.createPacketAndSend(BROADCAST_ADDR, helloPacket, 1);

    //     // Wait 20 seconds to send the next packet
    //     vTaskDelay(20000 / portTICK_PERIOD_MS);
    // }
     Serial.println("initBoard");
     vTaskDelay(2000 / portTICK_PERIOD_MS);
}