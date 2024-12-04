#include "targets.h"
#include "options.h"
#include "logging.h"
#define QUOTE(arg) #arg
#define STR(macro) QUOTE(macro)
const unsigned char target_name[] = "\xBE\xEF\xCA\xFE" STR(TARGET_NAME);
const uint8_t target_name_size = sizeof(target_name);

#ifdef LORA_GATEWAY
const char *wifi_hostname = "lora_gateway";
const char *wifi_ap_ssid = "Loramesh";
#else
const char *wifi_hostname = "lora_node";
const char *wifi_ap_ssid = "Loramesh";
#endif

const char *wifi_ap_password = "Loramesh";
const char *wifi_ap_address = "10.0.0.1";

#include <ArduinoJson.h>
#include <StreamString.h>
#if defined(PLATFORM_ESP8266)
#include <FS.h>
#else
#include <SPIFFS.h>
#endif
#if defined(PLATFORM_ESP32)
#include <esp_partition.h>
#include "esp_ota_ops.h"
#endif

char product_name[LORAOPTS_PRODUCTNAME_SIZE + 1];
char device_name[LORAOPTS_DEVICENAME_SIZE + 1];
uint32_t logo_image;

firmware_options_t firmwareOptions;

// hardware_init prototype here as it is called by options_init()
extern bool hardware_init(EspFlashStream &strmFlash);

static StreamString builtinOptions;
String &getOptions()
{
    return builtinOptions;
}

void saveOptions(Stream &stream, bool customised)
{
    JsonDocument doc;

    // UID
    if (firmwareOptions.hasUID)
    {
        JsonArray uid = doc["uid"].to<JsonArray>();
        copyArray(firmwareOptions.uid, sizeof(firmwareOptions.uid), uid);
    }

    // WiFi
    if (firmwareOptions.wifi_auto_on_interval != -1)
    {
        doc["wifi-on-interval"] = firmwareOptions.wifi_auto_on_interval / 1000;
    }
    if (firmwareOptions.home_wifi_ssid[0])
    {
        doc["wifi-ssid"] = firmwareOptions.home_wifi_ssid;
        doc["wifi-password"] = firmwareOptions.home_wifi_password;
    }

    // General settings
    doc["domain"] = firmwareOptions.domain;
    doc["customised"] = customised;
    doc["flash-discriminator"] = firmwareOptions.flash_discriminator;

    // // MQTT settings
    if (firmwareOptions.mqtt_server[0])
    {
        doc["mqtt-server"] = firmwareOptions.mqtt_server;
        doc["mqtt-port"] = firmwareOptions.mqtt_port;
        doc["mqtt-username"] = firmwareOptions.mqtt_username;
        doc["mqtt-password"] = firmwareOptions.mqtt_password;
        doc["mqtt-topic-sub"] = firmwareOptions.mqtt_topic_sub;
        doc["mqtt-topic-out"] = firmwareOptions.mqtt_topic_out;
        doc["mqtt-max-packet-size"] = firmwareOptions.mqtt_max_packet_size;
        doc["mqtt-max-queue-size"] = firmwareOptions.mqtt_max_queue_size;
        doc["mqtt-still-connected-interval"] = firmwareOptions.mqtt_still_connected_interval;
    }

    // Serialize to stream
    serializeJson(doc, stream);
    serializeJson(doc, Serial);
}

void saveOptions()
{
    File options = SPIFFS.open("/options.json", "w");
    saveOptions(options, true);
    options.close();
}

/**
 * @brief:  Checks if the strmFlash currently is pointing to something that looks like
 *          a string (not all 0xFF). Position in the stream will not be changed.
 * @return: true if appears to have a string
 */
bool options_HasStringInFlash(EspFlashStream &strmFlash)
{
    uint32_t firstBytes;
    size_t pos = strmFlash.getPosition();
    strmFlash.readBytes((uint8_t *)&firstBytes, sizeof(firstBytes));
    strmFlash.setPosition(pos);

    return firstBytes != 0xffffffff;
}
/**
 * @brief:  Internal read options from either the flash stream at the end of the sketch or the options.json file
 *          Fills the firmwareOptions variable
 * @return: true if either was able to be parsed
 */
static void options_LoadFromFlashOrFile(EspFlashStream &strmFlash)
{
    JsonDocument flashDoc;
    JsonDocument spiffsDoc;
    bool hasFlash = false;
    bool hasSpiffs = false;

    // Try OPTIONS JSON at the end of the firmware, after PRODUCTNAME DEVICENAME
    constexpr size_t optionConfigOffset = LORAOPTS_PRODUCTNAME_SIZE + LORAOPTS_DEVICENAME_SIZE;
    strmFlash.setPosition(optionConfigOffset);
    if (options_HasStringInFlash(strmFlash))
    {
        DeserializationError error = deserializeJson(flashDoc, strmFlash);
        if (error)
        {
            return;
        }
        hasFlash = true;
    }

    // Load options.json from the SPIFFS partition
    File file = SPIFFS.open("/options.json", "r");
    if (file && !file.isDirectory())
    {
        DeserializationError error = deserializeJson(spiffsDoc, file);
        if (!error)
        {
            hasSpiffs = true;
        }
    }

    JsonDocument &doc = flashDoc;
    if (hasFlash && hasSpiffs)
    {
        if (flashDoc["flash-discriminator"] == spiffsDoc["flash-discriminator"])
        {
            doc = spiffsDoc;
        }
    }
    else if (hasSpiffs)
    {
        doc = spiffsDoc;
    }

    // Load UID
    if (doc["uid"].is<JsonArray>())
    {
        copyArray(doc["uid"], firmwareOptions.uid, sizeof(firmwareOptions.uid));
        firmwareOptions.hasUID = true;
    }
    else
    {
        firmwareOptions.hasUID = false;
    }

    // Load WiFi settings
    int32_t wifiInterval = doc["wifi-on-interval"] | -1;
    firmwareOptions.wifi_auto_on_interval = wifiInterval == -1 ? -1 : wifiInterval * 1000;
    strlcpy(firmwareOptions.home_wifi_ssid, doc["wifi-ssid"] | "", sizeof(firmwareOptions.home_wifi_ssid));
    strlcpy(firmwareOptions.home_wifi_password, doc["wifi-password"] | "", sizeof(firmwareOptions.home_wifi_password));

    // Load domain and discriminator
    firmwareOptions.domain = doc["domain"] | 0;
    firmwareOptions.flash_discriminator = doc["flash-discriminator"] | 0U;

    // Load MQTT settings
    strlcpy(firmwareOptions.mqtt_server, doc["mqtt-server"] | "", sizeof(firmwareOptions.mqtt_server));
    firmwareOptions.mqtt_port = doc["mqtt-port"] | 1883;
    strlcpy(firmwareOptions.mqtt_username, doc["mqtt-username"] | "", sizeof(firmwareOptions.mqtt_username));
    strlcpy(firmwareOptions.mqtt_password, doc["mqtt-password"] | "", sizeof(firmwareOptions.mqtt_password));
    strlcpy(firmwareOptions.mqtt_topic_sub, doc["mqtt-topic-sub"] | "", sizeof(firmwareOptions.mqtt_topic_sub));
    strlcpy(firmwareOptions.mqtt_topic_out, doc["mqtt-topic-out"] | "", sizeof(firmwareOptions.mqtt_topic_out));
    firmwareOptions.mqtt_max_packet_size = doc["mqtt-max-packet-size"] | 512;
    firmwareOptions.mqtt_max_queue_size = doc["mqtt-max-queue-size"] | 10;
    firmwareOptions.mqtt_still_connected_interval = doc["mqtt-still-connected-interval"] | 300000;

    // Save built-in options
    builtinOptions.clear();
    saveOptions(builtinOptions, doc["customised"] | false);
}

void options_SetTrueDefaults()
{
    JsonDocument doc;
    // The Regulatory Domain is retained, as there is no sensible default
    doc["domain"] = firmwareOptions.domain;
    doc["flash-discriminator"] = firmwareOptions.flash_discriminator;

    File options = SPIFFS.open("/options.json", "w");
    serializeJson(doc, options);
    options.close();
}

/**
 * @brief:  Initializes product_name / device_name either from flash or static values
 * @return: true if the names came from flash, or false if the values are default
 */
static bool options_LoadProductAndDeviceName(EspFlashStream &strmFlash)
{
    if (options_HasStringInFlash(strmFlash))
    {
        strmFlash.setPosition(0);
        // Product name
        strmFlash.readBytes(product_name, LORAOPTS_PRODUCTNAME_SIZE);
        product_name[LORAOPTS_PRODUCTNAME_SIZE] = '\0';
        // Device name
        strmFlash.readBytes(device_name, LORAOPTS_DEVICENAME_SIZE);
        device_name[LORAOPTS_DEVICENAME_SIZE] = '\0';

        return true;
    }
    else
    {
        strcpy(product_name, "Undefied LORA NODE");
        strncpy(device_name, "Undefied LORA NODE", sizeof(device_name) - 1);
        device_name[sizeof(device_name) - 1] = '\0'; // Đảm bảo chuỗi kết thúc đúng

        return false;
    }
}
bool options_init()
{
    // debugCreateInitLogger();

    uint32_t baseAddr = 0;
#if defined(PLATFORM_ESP32)
    SPIFFS.begin(true);
    const esp_partition_t *runningPart = esp_ota_get_running_partition();
    if (runningPart)
    {
        baseAddr = runningPart->address;
    }
#else
    SPIFFS.begin();
    // ESP8266 sketch baseAddr is always 0
#endif
    EspFlashStream strmFlash;
    strmFlash.setBaseAddress(baseAddr + ESP.getSketchSize());

    // Product / Device Name
    options_LoadProductAndDeviceName(strmFlash);
    // options.json
    options_LoadFromFlashOrFile(strmFlash);
    // hardware.json
    bool hasHardware = hardware_init(strmFlash);
    logo_image = baseAddr + ESP.getSketchSize() +
                 LORAOPTS_PRODUCTNAME_SIZE +
                 LORAOPTS_DEVICENAME_SIZE +
                 LORAOPTS_OPTIONS_SIZE +
                 LORAOPTS_HARDWARE_SIZE;

    // debugFreeInitLogger();

    return hasHardware;
}
