#include "lora_common.h"
#include "options.h"
#include "devLED.h"
#include "devWIFI.h"
#include "PWM.h"
#if defined(PLATFORM_ESP32)
#include "devScreen.h"
#include "devETH.h"
#endif
// Manager
#include "messageManager.h"

// LoRaMesh
#include "loraMeshService.h"

static const char *TAG = "Main";
unsigned long rebootTime = 0;
device_affinity_t devices[] = {
    {&RGB_device, 0},
    {&WIFI_device, 0},
    {&Screen_device, 0},
    {&eth_device,1}};

Stream *NodeUSB;
Stream *NodeBackpack;
/**
 * Target-specific initialization code called early in setup()
 * Setup GPIOs or other hardware, config not yet loaded
 ***/

#pragma region LoRaMesher
LoRaMeshService &loraMeshService = LoRaMeshService::getInstance();

void initLoRaMesher()
{
    // Init LoRaMesher
    loraMeshService.initLoraMesherService();
}

#pragma endregion
#pragma region Manager

MessageManager &manager = MessageManager::getInstance();

void initManager()
{
    manager.init();
    ESP_LOGV(TAG, "Manager initialized");
    manager.addMessageService(&loraMeshService);
    ESP_LOGV(TAG, "LoRaMesher service added to manager");
}
#pragma endregion

#if defined(PLATFORM_ESP32_S3)
#include "USB.h"
#define USBSerial Serial
#endif

void setupSerial()
{
#if defined(PLATFORM_ESP32)
    Stream *serialPort;
    if (GPIO_PIN_DEBUG_RX != UNDEF_PIN && GPIO_PIN_DEBUG_TX != UNDEF_PIN)
    {
        serialPort = new HardwareSerial(0);
        ((HardwareSerial *)serialPort)->begin(BACKPACK_LOGGING_BAUD, SERIAL_8N1, GPIO_PIN_DEBUG_RX, GPIO_PIN_DEBUG_TX);
    }
    else
    {
        serialPort = new NullStream();
    }
#elif defined(PLATFORM_ESP8266)
    Stream *serialPort;
    if (GPIO_PIN_DEBUG_TX != UNDEF_PIN)
    {
        serialPort = new HardwareSerial(1);
        ((HardwareSerial *)serialPort)->begin(BACKPACK_LOGGING_BAUD, SERIAL_8N1, SERIAL_TX_ONLY, GPIO_PIN_DEBUG_TX);
    }
    else
    {
        serialPort = new NullStream();
    }
#endif
    NodeBackpack = serialPort;
#if defined(PLATFORM_ESP32_S3)
    Serial.begin(115200);
#endif

// Setup NodeUSB
#if defined(PLATFORM_ESP32_S3)
    USBSerial.begin(firmwareOptions.uart_baud);
    NodeUSB = &USBSerial;
#elif defined(PLATFORM_ESP32)
    if (GPIO_PIN_DEBUG_RX == 3 && GPIO_PIN_DEBUG_TX == 1)
    {
        // The backpack is already assigned on UART0 (pins 3, 1)
        // This is also USB on modules that use DIPs
        // Set NodeUSB to TxBackpack so that data goes to the same place
        NodeUSB = NodeBackpack;
    }
    else if (GPIO_PIN_RCSIGNAL_RX == U0RXD_GPIO_NUM && GPIO_PIN_RCSIGNAL_TX == U0TXD_GPIO_NUM)
    {
        // This is an internal module, or an external module configured with a relay.  Do not setup NodeUSB.
        NodeUSB = new NullStream();
    }
    else
    {
        // The backpack is on a separate UART to UART0
        // Set NodeUSB to pins 3, 1 so that we can access NodeUSB and TxBackpack independantly
        NodeUSB = new HardwareSerial(1);
        ((HardwareSerial *)NodeUSB)->begin(firmwareOptions.uart_baud, SERIAL_8N1, 3, 1);
    }
#else
    NodeUSB = new NullStream();
#endif
}
static void setupTarget()
{
    setupSerial();
    setupTargetCommon();
}
bool setupHardwareFromOptions()
{
    if (!options_init())
    {
        // Register the WiFi with the framework
        NodeBackpack->println("Wifi start");
        static device_affinity_t wifi_device[] = {
            {&WIFI_device, 1}};
        devicesRegister(wifi_device, ARRAY_SIZE(wifi_device));
        devicesInit();
        return false;
    }
    return true;
}
void setup()
{
    if (setupHardwareFromOptions())
    {
        setupTarget();
        devicesRegister(devices, ARRAY_SIZE(devices));
        devicesInit();
        devicesStart();
    }
    pwm.init_pwm();
    // Initialize Manager
    initManager();

    ESP_LOGV(TAG, "Heap after initManager: %d", ESP.getFreeHeap());
    initLoRaMesher();
    ESP_LOGV(TAG, "Heap after initLoRaMesher: %d", ESP.getFreeHeap());
    NodeUSB->print("Commnad:");
    NodeUSB->println(manager.getAvailableCommands());
}
#include "WiFi.h"
void loop()
{
    unsigned long now = millis();
    devicesUpdate(now);
    if (rebootTime != 0 && now > rebootTime)
    {
        ESP.restart();
    }
}
