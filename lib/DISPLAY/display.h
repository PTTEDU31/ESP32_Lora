#pragma once
#include
#include "targets.h"
#include "dataMessage.h"
#ifndef USE_OLED_I2C
#define OPT_USE_OLED_I2C false
#elif !defined(OPT_USE_OLED_I2C)
#define OPT_USE_OLED_I2C true
#endif
#ifndef USE_OLED_SPI
#define OPT_USE_OLED_SPI false
#elif !defined(OPT_USE_OLED_SPI)
#define OPT_USE_OLED_SPI true
#endif
#ifndef USE_OLED_SPI_SMALL
#define OPT_USE_OLED_SPI_SMALL false
#elif !defined(OPT_USE_OLED_SPI_SMALL)
#define OPT_USE_OLED_SPI_SMALL true
#endif
#ifndef OLED_REVERSED
#define OPT_OLED_REVERSED false
#elif !defined(OPT_OLED_REVERSED)
#define OPT_OLED_REVERSED true
#endif
#ifndef HAS_TFT_SCREEN
#define OPT_HAS_TFT_SCREEN false
#elif !defined(OPT_HAS_TFT_SCREEN)
#define OPT_HAS_TFT_SCREEN true
#endif


class Display
{
    virtual void init() = 0;
    
}
enum DisplayCommand: uint8_t {
    DisplayOn = 0,
    DisplayOff = 1,
    DisplayBlink = 2,
    DisplayClear = 3,
    DisplayText = 4,
    DisplayLogo = 5
};

class DisplayMessage: public DataMessageGeneric {
public:
    DisplayCommand displayCommand;
    union {
        char displayText[128];
    };

    uint32_t getDisplayTextSize() {
        return this->messageSize - sizeof(DisplayCommand);
    }

    void serialize(JsonObject& doc) {
        ESP_LOGE(DISPLAY_TAG, "Display Message not implemented");
    }

    void deserialize(JsonObject& doc) {
        // Call the base class deserialize function
        ((DataMessageGeneric*) (this))->deserialize(doc);

        // Add the derived class data to the JSON object
        displayCommand = doc["displayCommand"];

        size_t displayTextSize = 0;

        switch (displayCommand) {
            case DisplayText: {
                    displayTextSize = strlen(doc["displayText"]);
                    if (displayTextSize > 32) {
                        ESP_LOGE(DISPLAY_TAG, "displayText is too long");
                        return;
                    }
                    strcpy(displayText, doc["displayText"]);
                    break;
                }
            default:
                break;
        }

        this->messageSize = displayTextSize + sizeof(DisplayCommand);

    }
};