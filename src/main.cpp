#include "lora_common.h"
#include "options.h"
#include "devLED.h"
#include "devWIFI.h"
device_affinity_t devices[] = {
    {&RGB_device, 1},
    {&WIFI_device,1}
};

Stream *NodeUSB;
Stream *NodeBackpack;
/**
 * Target-specific initialization code called early in setup()
 * Setup GPIOs or other hardware, config not yet loaded
 ***/

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

// Setup TxUSB
#if defined(PLATFORM_ESP32_S3)
    USBSerial.begin(firmwareOptions.uart_baud);
    NodeUSB = &USBSerial;
#elif defined(PLATFORM_ESP32)
    if (GPIO_PIN_DEBUG_RX == 3 && GPIO_PIN_DEBUG_TX == 1)
    {

        NodeUSB = NodeBackpack;
    }
    else if (GPIO_PIN_RCSIGNAL_RX == U0RXD_GPIO_NUM && GPIO_PIN_RCSIGNAL_TX == U0TXD_GPIO_NUM)
    {
        NodeUSB = new NullStream();
    }
    else
    {
        // The backpack is on a separate UART to UART0
        // Set TxUSB to pins 3, 1 so that we can access TxUSB and TxBackpack independantly
        NodeUSB = new HardwareSerial(1);
        ((HardwareSerial *)TxUSB)->begin(firmwareOptions.uart_baud, SERIAL_8N1, 3, 1);
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

        connectionState = hardwareUndefined;
        return false;
    }
    return true;
}
void setup()
{
    if (setupHardwareFromOptions())
    {

        setupTarget();
        NodeBackpack->printf("\nGPRS_Rx: %d  GPRS_Tx: %d Baud:%d", GPIO_PIN_GPRS_RX, GPIO_PIN_GPRS_TX, GPRS_BAUD);
        NodeBackpack->printf("\nRS485_Rx: %d  GPRS_Tx: %d Baud:%d", GPIO_PIN_RS485_TX, GPIO_PIN_RS485_RX, RS485_BAUD);
        NodeBackpack->printf("\nWifi_SSID: %s, Wifi_Pass: %s", firmwareOptions.home_wifi_ssid, firmwareOptions.home_wifi_password);
        NodeBackpack->print(getOptions());

        devicesRegister(devices, ARRAY_SIZE(devices)); // Đăng ký 2 thiết bị
        devicesInit();
        devicesStart();
        connectionState = wifiUpdate;
    }
}

void loop()
{
    // NodeBackpack.printf("TX_RX_PIN %d", GPIO_PIN_RCSIGNAL_TX);
    unsigned long now = millis();
    devicesUpdate(now);
    // Serial.println(xPortGetCoreID());
}
