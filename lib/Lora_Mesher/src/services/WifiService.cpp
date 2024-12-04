#include "WiFiService.h"
#include "esp_mac.h"
#include "WiFi.h"
void WiFiService::init()
{
    uint8_t baseMac[6];
// Get MAC address for WiFi station
// esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
// char baseMacChr[18] = {0};
// sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
// localAddress = (baseMac[4] << 8) | baseMac[5];
#ifdef ARDUINO
    WiFi.macAddress(baseMac);
#else
    efuse_hal_get_mac(baseMac);
#endif
    localAddress = (baseMac[4] << 8) | baseMac[5];
    ESP_LOGI(LM_TAG, "Local LoRa address (from WiFi MAC): %X", localAddress);
}

uint16_t WiFiService::getLocalAddress()
{
    if (localAddress == 0)
        init();
    return localAddress;
}

uint16_t WiFiService::localAddress = 0;
