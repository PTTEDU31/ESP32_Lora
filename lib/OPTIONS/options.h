#pragma once

#include "targets.h"

extern const unsigned char target_name[];
extern const uint8_t target_name_size;

extern const char *wifi_hostname;
extern const char *wifi_ap_ssid;
extern const char *wifi_ap_password;
extern const char *wifi_ap_address;

typedef struct _options
{
    uint8_t     _magic_[8];     // This is the magic constant so the configurator can find this options block
    uint16_t    _version_;      // The version of this structure
    uint8_t     domain;         // Depends on radio chip
    uint8_t     hasUID;
    uint8_t     uid[16];        // MY_UID derived from MY_BINDING_PHRASE
    uint32_t    flash_discriminator;    // Discriminator value used to determine if the device has been reflashed and therefore
                                        // the SPIFSS settings are obsolete and the flashed settings should be used in preference
    uint32_t    fan_min_runtime;

#if defined(PLATFORM_ESP32) || defined(PLATFORM_ESP8266)
    int32_t wifi_auto_on_interval;
    char home_wifi_ssid[33];
    char home_wifi_password[65];
#endif

    uint32_t uart_baud;

    // MQTT configuration
    char mqtt_server[65];       // MQTT server address (e.g., "192.168.1.26")
    uint16_t mqtt_port;         // MQTT server port (e.g., 1883)
    char mqtt_username[33];     // MQTT username
    char mqtt_password[65];     // MQTT password
    char mqtt_topic_sub[65];    // MQTT topic for subscribing
    char mqtt_topic_out[65];    // MQTT topic for publishing
    uint32_t mqtt_max_packet_size; // Maximum packet size
    uint32_t mqtt_max_queue_size;  // Maximum queue size
    uint32_t mqtt_still_connected_interval; // Interval for checking MQTT connection (ms)
    
} __attribute__((packed)) firmware_options_t;

constexpr size_t LORAOPTS_PRODUCTNAME_SIZE = 128;
constexpr size_t LORAOPTS_DEVICENAME_SIZE = 16;
constexpr size_t LORAOPTS_OPTIONS_SIZE = 1024;
constexpr size_t LORAOPTS_HARDWARE_SIZE = 2048;

extern firmware_options_t firmwareOptions;
extern char product_name[];
extern char device_name[];
extern uint32_t logo_image;
extern bool options_init();
extern String &getOptions();
extern String &getHardware();
extern void saveOptions();

#include "EspFlashStream.h"
bool options_HasStringInFlash(EspFlashStream &strmFlash);
void options_SetTrueDefaults();