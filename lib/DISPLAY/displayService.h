#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "messageService.h"

#include "messageManager.h"

#include "displayCommandService.h"

#include "displayMessage.h"

#include "config.h"

#include "LoraMesher.h"
#include "OLED/oleddisplay.h"

class DisplayService: public MessageService {
public:
    static DisplayService& getInstance() {
        static DisplayService instance;
        return instance;
    }

    DisplayCommandService* displayCommandService = new DisplayCommandService();

    void init();

    String displayOn(uint16_t dst);

    String displayOff(uint16_t dst);

    String displayBlink(uint16_t dst);

    String clearDisplay(uint16_t dst);

    String displayLogo(uint16_t dst, uint16_t src = 0);

    String displayText(uint16_t dst, String text, uint16_t src = 0);

    String getJSON(DataMessage* message);

    DataMessage* getDataMessage(JsonObject data);

    DataMessage* getDisplayMessage(DisplayCommand command, uint16_t dst, String text = "");

    void processReceivedMessage(messagePort port, DataMessage* message);

    void printGPSData(String data);


private:
    DisplayService(): MessageService(DisplayApp, "Display") {
        commandService = displayCommandService;
    };
    U8G2 *u8g2;
    TaskHandle_t display_TaskHandle = NULL;

    void createDisplayTask();

    static void displayTask(void* pvParameters);

    std::vector<String> displayTextVector;
    std::vector<int> xPos;
    std::vector<int> minXPos;
    std::vector<bool> moveStatus;
    const int maxLines = 7; // Maximum number of lines the display can handle
    const int lineHeight = 9; // Height of each line of text
    const int staticLines = 3; // Number of static lines at the top

    void drawDisplay();
    void printLine(const String& str, int& x, int y, int size, int minX, bool move);
    void addText(String text);
    void setupTextMovement(int line, const String& text);
    void setTitle();

    bool displayOnFlag = true;
    bool displayingLogo = false;
    bool initialized = false;
};