#pragma once

#include "targets.h"
// #include "menu.h"

// #define CHANGED_TEMP bit(0)
// #define CHANGED_RATE bit(1)
// #define CHANGED_POWER bit(2)
// #define CHANGED_TELEMETRY bit(3)
// #define CHANGED_MOTION bit(4)
// #define CHANGED_FAN bit(5)
// #define CHANGED_ALL 0xFF

// typedef enum fsm_state_s menu_item_t;

// typedef enum message_index_e {
//     MSG_NONE,
//     MSG_CONNECTED,
//     MSG_ARMED,
//     MSG_MISMATCH,
//     MSG_ERROR,
//     MSG_INVALID
// } message_index_t;

// typedef enum
// {
//     SCREEN_BACKLIGHT_ON = 0,
//     SCREEN_BACKLIGHT_OFF = 1
// } screen_backlight_t;

class Display
{
public:
    virtual void init() = 0;
    virtual void display_print_addr() = 0 ;
    virtual void display_wifi_status() = 0;
    
protected:
    static const char *message_string[];
    static const char *main_menu_strings[][2];
};
