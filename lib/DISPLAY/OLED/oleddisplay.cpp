#if defined(PLATFORM_ESP32)

#include <U8g2lib.h> // Needed for the OLED drivers, this is an Arduino package. It is maintained by PlatformIO
#include "oleddisplay.h"
#include "options.h"
#include "logging.h"
#include "common.h"
#include "loraMeshService.h"
#include "WiFi.h"
extern WiFiMode_t wifiMode;
uint64_t count = 0 ;
// OLED specific header files.
U8G2 *u8g2;

void OLEDDisplay::init()
{
    if (OPT_HAS_OLED_SPI_SMALL)
        u8g2 = new U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI(OPT_SCREEN_REVERSED ? U8G2_R2 : U8G2_R0, GPIO_PIN_SCREEN_SCK, GPIO_PIN_SCREEN_MOSI, GPIO_PIN_SCREEN_CS, GPIO_PIN_SCREEN_DC, GPIO_PIN_SCREEN_RST);
    else if (OPT_HAS_OLED_SPI)
        u8g2 = new U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI(OPT_SCREEN_REVERSED ? U8G2_R2 : U8G2_R0, GPIO_PIN_SCREEN_SCK, GPIO_PIN_SCREEN_MOSI, GPIO_PIN_SCREEN_CS, GPIO_PIN_SCREEN_DC, GPIO_PIN_SCREEN_RST);
    else if (OPT_HAS_OLED_I2C)
        u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(OPT_SCREEN_REVERSED ? U8G2_R2 : U8G2_R0, GPIO_PIN_SCREEN_RST, GPIO_PIN_SCREEN_SCK, GPIO_PIN_SCREEN_SDA);

    u8g2->begin();
}

void OLEDDisplay::display_print_addr()
{
    char macStr[5];
    snprintf(macStr, sizeof(macStr), "%04X", LoRaMeshService::getInstance().getLocalAddress());
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_ncenB14_tr);
    u8g2->setCursor(20, 20);
    u8g2->print(macStr);
    u8g2->sendBuffer();
}

void OLEDDisplay::display_wifi_status()
{
    char macStr[5];
    snprintf(macStr, sizeof(macStr), "%04X", LoRaMeshService::getInstance().getLocalAddress());
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_6x10_tr);

    if (WiFi.isConnected())
    {
        char ssid[33];
        char ip[16];

        // Get WiFi SSID and IP
        strncpy(ssid, WiFi.SSID().c_str(), sizeof(ssid) - 1);
        ssid[sizeof(ssid) - 1] = '\0';
        snprintf(ip, sizeof(ip), "%s", WiFi.localIP().toString().c_str());

        // Display WiFi SSID
        u8g2->setCursor(0, 12);
        u8g2->print("SSID:");
        u8g2->setCursor(40, 12);
        u8g2->print(ssid);

        // Display IP address
        u8g2->setCursor(0, 24);
        u8g2->print("IP:");
        u8g2->setCursor(40, 24);
        u8g2->print(ip);
    }
    else
    {
        // Display WiFi disconnected message
        u8g2->setCursor(0, 12);
        u8g2->print("WiFi: Not connected");
    }
    u8g2->setCursor(0, 36);
    u8g2->print(count++);
    u8g2->setCursor(60, 36);
    u8g2->print(GATEWAY?"GATEWAY":"NODE");
    u8g2->setCursor(0, 48);
    u8g2->print(macStr);
    u8g2->setCursor(30, 48);
    u8g2->print(connectionState);
    u8g2->setCursor(60, 48);
    u8g2->print(ESP.getFreeHeap());
    u8g2->setCursor(60, 64);
    u8g2->print(ESP.getFreePsram());
    u8g2->sendBuffer();

}
#endif
